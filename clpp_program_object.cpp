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

#include "CLplusplus/device.hpp"
#include "CLplusplus/platform.hpp"
#include "CLplusplus/program.hpp"
#include "CLplusplus/version.hpp"

#include "shared.hpp"

// This program demonstrates basic program object manipulation in CLplusplus
int main() {
   // Minimal platform and device parameters are specified here
   const CLplusplus::Version target_version = CLplusplus::version_1p2;

   // Have the user select a suitable device, according to some criteria (see shared.hpp for more details)
   const auto selected_platform_and_device = Shared::select_device(
      [&](const CLplusplus::Platform & platform) -> bool {
         return (platform.version() >= target_version);                       // Platform OpenCL version is recent enough
      },
      [&](const CLplusplus::Device & device) -> bool {
         if(device.version() < target_version) return false;                  // OpenCL platforms may support older-generation devices, which we need to eliminate
         const bool device_supports_ooe_execution = device.queue_properties() & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
         return device.available() &&                                         // Device is available for compute purposes
                (device.execution_capabilities() & CL_EXEC_KERNEL) &&         // Device can execute OpenCL kernels
                device_supports_ooe_execution &&                              // Device can execute OpenCL commands out of order
                device.compiler_available() && device.linker_available();     // Implementation has an OpenCL C compiler and linker for this device
      }
   );

   // Create an OpenCL context on the device with some default parameters (see shared.hpp for more details)
   const auto context = Shared::build_default_context(selected_platform_and_device);

   // Create a program object from the basic vector addition example
   auto program = context.create_program_with_source_file("vector_add.cl");

   // Synchronously build it
   program.build("-cl-mad-enable -cl-no-signed-zeros -cl-std=CL1.2 -cl-kernel-arg-info");

   // Check out its properties
   std::cout << "Program object is associated to " << program.num_devices() << " device(s):" << std::endl;
   for(const auto & device : program.devices()) std::cout << " - " << device.name() << std::endl;
   std::cout << std::endl;

   std::cout << "=== Program source code follows ===" << std::endl;
   std::cout << program.source();
   std::cout << "===================================" << std::endl << std::endl;

   const auto binary_sizes = program.binary_sizes();
   std::cout << "Program has " << binary_sizes.size() << " associated binaries, of size(s) ";
   for(const auto bin_size : binary_sizes) std::cout << bin_size << ' ';
   std::cout << std::endl;

   const auto binaries = program.binaries();
   std::cout << "I can successfully fetch all binaries (count: " << binaries.size() << ')' << std::endl;

   std::cout << "Program has " << program.num_kernels() << " associated kernels: ";
   for(const auto & kernel_name : program.kernel_names()) std::cout << kernel_name << ' ';
   std::cout << std::endl;

   // Display device-specific build information
   std::cout << std::endl;
   const auto device = selected_platform_and_device.second;
   std::cout << "Now, what do we know about the build results for the " << device.name() << "?" << std::endl;

   switch(program.build_status(device)) {
      case CL_BUILD_NONE:
         std::cout << "Strangely enough, it seems our program has never been built for this device";
         break;
      case CL_BUILD_ERROR:
         std::cout << "The program build has failed, and none told us about it!";
         break;
      case CL_BUILD_SUCCESS:
         std::cout << "The program has been successfully built, as expected";
         break;
      case CL_BUILD_IN_PROGRESS:
         std::cout << "The program is not actually built yet!";
         break;
      default:
         std::cout << "The program has an UNKNOWN build status!";
   }
   std::cout << std::endl;

   std::cout << "The build options are : " << program.build_options(device) << std::endl;

   const auto build_log = program.build_log(device);
   if(build_log.length() <= 2) {
      std::cout << "The implementation returned no build log for this device" << std::endl;
   } else {
      std::cout << "The implementation returned the following build log:" << std::endl;
      std::cout << "---------------" << std::endl;
      std::cout << program.build_log(device);
      std::cout << "---------------" << std::endl;
   }

   switch(program.binary_type(device)) {
      case CL_PROGRAM_BINARY_TYPE_NONE:
         std::cout << "The program has no binary associated to it";
         break;
      case CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT:
         std::cout << "The program binary is an object file, suitable for further linking";
         break;
      case CL_PROGRAM_BINARY_TYPE_LIBRARY:
         std::cout << "The program binary is a library file, suitable for further linking";
         break;
      case CL_PROGRAM_BINARY_TYPE_EXECUTABLE:
         std::cout << "The program binary is an executable file, ready to be turned into a kernel!";
         break;
      default:
         std::cout << "The program is associated to an UNKNOWN kind of binary!";
   }
   std::cout << std::endl;

   return 0;
}
