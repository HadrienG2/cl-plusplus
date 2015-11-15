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

#include <CL/cl.h>

#include "CLplusplus/command_queue.hpp"
#include "CLplusplus/device.hpp"
#include "CLplusplus/platform.hpp"
#include "CLplusplus/version.hpp"

#include "shared.hpp"

// This program demonstrates conmmand queue creation and basic handling in CLplusplus
int main() {
   // Some minimal platform and device parameters are specified here
   const CLplusplus::Version target_version = CLplusplus::version_1p2;
   const cl_ulong min_mem_alloc_size = 20 * 1024 * 1024;
   const cl_ulong min_local_mem_size = 16 * 1024;

   // Have the user select a suitable device, according to some criteria (see shared.hpp for more details)
   // TODO : Transfer some of this complexity to more advanced CLplusplus examples
   const auto selected_platform_and_device = Shared::select_device(
      [&](const CLplusplus::Platform & platform) -> bool {
         return (platform.version() >= target_version);                       // Platform OpenCL version is recent enough
      },
      [&](const CLplusplus::Device & device) -> bool {
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

   // Create an OpenCL context on the device with some default parameters (see shared.hpp for more details)
   const auto context = Shared::build_default_context(selected_platform_and_device);

   // Create an out-of-order command queue for the device
   const auto command_queue = context.create_command_queue(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

   // Display command queue properties
   if(command_queue.raw_context_id() != context.raw_identifier()) std::cout << "Oops ! Command queue seems to identify with the wrong context..." << std::endl;

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

   // Try flushing and finishing our command queue. Should return without doing anything.
   command_queue.flush();
   command_queue.finish();

   return 0;
}
