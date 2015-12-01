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
#include "CLplusplus/kernel.hpp"
#include "CLplusplus/platform.hpp"
#include "CLplusplus/program.hpp"
#include "CLplusplus/version.hpp"

#include "shared.hpp"

// This program demonstrates basic kernel object manipulation in CLplusplus
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

   // Start an asynchronous program build
   const auto build_event = program.build_with_event("-cl-mad-enable -cl-no-signed-zeros -cl-std=CL1.2 -cl-kernel-arg-info");

   // NOTE : We could be doing stuff unrelated to the ongoing program build here, such as loading data into buffers

   // Once the program is built, create a kernel object associated to our vector addition routine
   const auto kernel = program.create_kernel("vector_add", build_event);

   // Check out its general properties
   std::cout << "Kernel is associated to the function " << kernel.function_name() << std::endl;

   std::cout << "Kernel has " << kernel.num_args() << " arguments" << std::endl;

   const auto attributes = kernel.attributes();
   if(attributes.size() == 0) {
      std::cout << "Kernel has no attributes" << std::endl;
   } else {
      std::cout << "Kernel attributes are:" << std::endl;
      for(const auto & attribute : attributes) std::cout << " - " << attribute << std::endl;
   }

   if(kernel.raw_context_id() == context.raw_identifier()) {
      std::cout << "Kernel identifies with the right context." << std::endl;
   } else {
      std::cout << "Uh oh, kernel identifies with an unknown context...";
   }

   if(kernel.raw_program_id() == program.raw_identifier()) {
      std::cout << "Kernel identifies with the right program." << std::endl;
   } else {
      std::cout << "Uh oh, kernel identifies with an unknown program...";
   }

   // Check out its device-specific properties
   const auto & device = selected_platform_and_device.second;
   std::cout << std::endl;
   std::cout << "Now, onto kernel properties which are specific to the " << device.name() << ":" << std::endl;

   std::cout << "For this kernel, work groups cannot be larger than " << kernel.work_group_size(device) << " items" << std::endl;

   const auto compile_wg_size = kernel.compile_work_group_size(device);
   if(compile_wg_size == std::array<size_t, 3>{0, 0, 0}) {
      std::cout << "This kernel does not require the use of a specific work group size" << std::endl;
   } else {
      std::cout << "This kernel must be launched with a work group size of (" << compile_wg_size[0] << ", " << compile_wg_size[1] << ", " << compile_wg_size[2] << ")" << std::endl;
   }

   std::cout << "This kernel will require at least " << kernel.local_mem_size(device) << " byte(s) of local memory" << std::endl;

   std::cout << "Implementation suggests that the work group size be a multiple of " << kernel.preferred_work_group_size_multiple(device) << std::endl;

   std::cout << "This kernel will require " << kernel.private_mem_size(device) << " byte(s) of private memory" << std::endl;

   // Check out the properties of its arguments
   std::cout << std::endl;
   std::cout << "This kernel has the following arguments (in order):" << std::endl;

   const auto num_args = kernel.num_args();
   for(size_t arg = 0; arg < num_args; ++arg) {
      std::cout << " - ";

      // Address space qualifiers
      switch(kernel.arg_address_qualifier(arg)) {
         case CL_KERNEL_ARG_ADDRESS_GLOBAL:
            std::cout << "__global ";
            break;
         case CL_KERNEL_ARG_ADDRESS_LOCAL:
            std::cout << "__local ";
            break;
         case CL_KERNEL_ARG_ADDRESS_CONSTANT:
            std::cout << "__constant ";
            break;
         case CL_KERNEL_ARG_ADDRESS_PRIVATE:
            break;
         default:
            std::cout << "<UNKNOWN> ";
      }

      // Image access qualifiers
      switch(kernel.arg_access_qualifier(arg)) {
         case CL_KERNEL_ARG_ACCESS_READ_ONLY:
            std::cout << "read_only ";
            break;
         case CL_KERNEL_ARG_ACCESS_WRITE_ONLY:
            std::cout << "write_only ";
            break;
         case CL_KERNEL_ARG_ACCESS_READ_WRITE:
         case CL_KERNEL_ARG_ACCESS_NONE:
            break;
         default:
            std::cout << "<UNKNOWN> ";
      }

      // CV type qualifiers
      const auto type_qualifier = kernel.arg_type_qualifier(arg);
      if(type_qualifier & CL_KERNEL_ARG_TYPE_CONST) std::cout << "const ";
      if(type_qualifier & CL_KERNEL_ARG_TYPE_VOLATILE) std::cout << "volatile ";

      // Unqualified type name
      const auto type_name = kernel.arg_type_name(arg);
      const bool is_ptr = (type_name[type_name.length() - 1] == '*');
      if(is_ptr) {
         std::cout << type_name.substr(0, type_name.length() - 1) << " * ";
      } else {
         std::cout << type_name << ' ';
      }

      // Restrict qualifier
      // NOTE: the OpenCL spec thinks that this one should go before a pointer type's star, but the C99 spec, the NVidia implementation and I disagree on this front)
      if(type_qualifier & CL_KERNEL_ARG_TYPE_RESTRICT) std::cout << "restrict ";

      // Argument name
      std::cout << kernel.arg_name(arg);

      std::cout << std::endl;
   }

   return 0;
}
