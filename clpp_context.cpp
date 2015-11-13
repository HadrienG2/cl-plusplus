// Copyright 2015 Hadrien Grasland
//
// This file is part of CLplusplus.
//
// CLplusplus is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CLplusplus is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CLplusplus.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <CL/cl.h>

#include "CLplusplus/context.hpp"
#include "CLplusplus/platform.hpp"
#include "CLplusplus/queries.hpp"
#include "CLplusplus/version.hpp"

// Some minimal platform and device parameters are specified here, but most of them are specified below
const CLplusplus::Version target_version = CLplusplus::version_1p2;
const cl_ulong min_mem_alloc_size = 20 * 1024 * 1024;
const cl_ulong min_local_mem_size = 16 * 1024;

// This program demonstrates context creation and basic handling in CLplusplus
int main() {
   // Detect OpenCL platform and device combinations which match our expectations
   const auto filtered_platforms = CLplusplus::get_filtered_devices(
      [](const CLplusplus::Platform & platform) -> bool {
         return (platform.version() >= target_version);                       // Platform OpenCL version is recent enough
      },
      [](const CLplusplus::Device & device) -> bool {
         if(device.version() < target_version) return false;                  // OpenCL platforms may support older-generation devices, which we need to eliminate
         const bool device_supports_ooe_execution = device.queue_properties() & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
         const auto device_double_config = device.double_fp_config();
         return device.available() &&                                         // Device is available for compute purposes
                device.endian_little() &&                                     // Device is little-endian
                (device.execution_capabilities() & CL_EXEC_KERNEL) &&         // Device can execute OpenCL kernels
                device_supports_ooe_execution &&                              // Device can execute OpenCL commands out of order
                device.compiler_available() && device.linker_available() &&   // Implementation has an OpenCL C compiler and linker for this device
                (device.max_mem_alloc_size() >= min_mem_alloc_size) &&        // Device accepts large enough global memory allocations
                (device.local_mem_type() == CL_LOCAL) &&                      // Device has local memory support, with dedicated storage
                (device.local_mem_size() >= min_local_mem_size) &&            // Device has a large enough local memory
                (device_double_config != 0) &&                                // Doubles are supported
                ((device_double_config & CL_FP_SOFT_FLOAT) == 0);             // Doubles are not emulated in software
      }
   );

   // Abort if no platform or device match our criteria
   if(filtered_platforms.size() == 0) {
      std::cout << "No suitable OpenCL platform or device detected!" << std::endl;
      std::abort();
   }

   // Construct and display a numbered list of suitable OpenCL devices
   std::cout << "Please pick an OpenCL device:" << std::endl;
   size_t device_count = 0, device_number;
   using PlatformAndDevice = std::pair<CLplusplus::Platform, CLplusplus::Device>;
   std::vector<PlatformAndDevice> possible_devices;
   for(const auto & filtered_platform : filtered_platforms) {
      const auto & platform = filtered_platform.platform;
      for(const auto & device : filtered_platform.filtered_devices) {
         possible_devices.emplace_back(PlatformAndDevice{platform, device});
         std::cout << " [" << device_count << "] " << device.name() << " (vendor ID " << device.vendor_id() << ", on platform " << platform.name() << ")" << std::endl;
         ++device_count;
      }
   }
   std::cout << std::endl;

   // Have the user select which device we should use
   std::cout << "Your choice > ";
   bool try_again;
   do {
      std::cin >> device_number;
      try_again = (std::cin.fail()) || (device_number >= device_count);
      if(try_again) {
         std::cout << "That wouldn't work. Please try again > ";
         std::cin.clear();
         std::cin.ignore();
      }
   } while(try_again);
   std::cout << std::endl;
   const PlatformAndDevice & selected_platform_and_device = possible_devices[device_number];

   // Create an OpenCL context on the selected device
   CLplusplus::ContextProperties context_properties;
   const auto platform_id = selected_platform_and_device.first.raw_platform_id();
   context_properties.append(CL_CONTEXT_PLATFORM, (cl_context_properties)(void *)platform_id);
   const auto context_callback = [](const std::string & errinfo, const void * private_info, size_t cb) -> void {
      std::cout << std::endl << "OPENCL CONTEXT ERROR: " << errinfo << " (private info at address " << (size_t)(void*)(private_info) << ", cb is " << cb << ")" << std::endl;
   };
   CLplusplus::Context context{context_properties, selected_platform_and_device.second, context_callback};

   // Display context properties
   std::cout << "Generated OpenCL context features " << context.num_devices() << " device(s) :" << std::endl;

   for(const auto & device : context.devices()) {
      std::cout << " * " << device.name() << " (vendor ID " << device.vendor_id() << ")" << std::endl;
   }
   
   std::cout << "The context was created with the following properties :" << std::endl;
   for(const auto & property : context.properties()) {
      switch(property.name()) {
         case CL_CONTEXT_PLATFORM:
            {
               const CLplusplus::Platform platform((cl_platform_id)(void *)property.value());
               std::cout << " * Platform is " << platform.name() << std::endl;
            }
            break;
         case CL_CONTEXT_INTEROP_USER_SYNC:
            {
               std::cout << " * In interop scenarii, ";
               const bool user_is_responsible = (property.value() == CL_TRUE);
               if(user_is_responsible) {
                  std::cout << "the user is responsible for OpenCL-graphics synchronization";
               } else {
                  std::cout << "OpenCL-graphics synchronization is managed by the platform";
               }
               std::cout << std::endl;
            }
            break;
         default:
            std::cout << " * <Some unrecognized property>" << std::endl;
      }
   }
   std::cout << std::endl;

   // Create an out-of-order command queue for the device
   const auto command_queue = context.create_command_queue(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

   // Display command queue properties
   if(command_queue.raw_context_id() != context.raw_context_id()) std::cout << "Oops ! Command queue seems to identify with the wrong context..." << std::endl;

   const auto queue_device = command_queue.device();
   std::cout << "Command queue device is " << queue_device.name() << " (vendor ID " << queue_device.vendor_id() << ")" << std::endl;

   const auto queue_properties = command_queue.properties();
   std::cout << "Command execution will be performed ";
   if(queue_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) {
      std::cout << "in-order";
   } else {
      std::cout << "out-of-order";
   }
   std::cout << std::endl;
   std::cout << "Command profiling is ";
   if(queue_properties & CL_QUEUE_PROFILING_ENABLE) {
      std::cout << "enabled";
   } else {
      std::cout << "disabled";
   }
   std::cout << std::endl;

   return 0;
}
