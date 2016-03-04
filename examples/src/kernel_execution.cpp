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
#include <vector>

#include <CL/cl.h>

#include "CLplusplus/device.hpp"
#include "CLplusplus/kernel.hpp"
#include "CLplusplus/platform.hpp"
#include "CLplusplus/program.hpp"
#include "CLplusplus/version.hpp"

#include "shared.hpp"

// This program implements a simple vector addition routine to demonstrate kernel execution
// It is mostly a straightforward port of the vector addition example from Heterogeneous Computing with OpenCL
int main() {
   // In this example, we will be summing two integer vectors of a hard-coded size
   const size_t vector_length = 64 * 1024 * 1024;
   const size_t vector_size = vector_length * sizeof(cl_int);

   // Minimal platform and device parameters are specified here
   const CLplusplus::Version target_version = CLplusplus::version_1p2;
   const cl_ulong min_mem_alloc_size = vector_size;
   const cl_ulong min_global_mem_size = 3 * vector_size;

   // Have the user select a suitable device, according to some criteria (see shared.hpp for more details)
   const auto selected_platform_and_device = Shared::select_device(
      [&](const CLplusplus::Platform & platform) -> bool {
         return (platform.version() >= target_version);                       // Platform OpenCL version is recent enough
      },
      [&](const CLplusplus::Device & device) -> bool {
         if(device.version() < target_version) return false;                  // OpenCL platforms may support older-generation devices, which we need to eliminate
         const bool device_supports_ooe_execution = device.queue_properties() & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
         return device.available() &&                                         // Device is available for compute purposes
                device.endian_little() &&                                     // Device is little-endian
                (device.execution_capabilities() & CL_EXEC_KERNEL) &&         // Device can execute OpenCL kernels
                device_supports_ooe_execution &&                              // Device can execute OpenCL commands out of order
                device.compiler_available() && device.linker_available() &&   // Implementation has an OpenCL C compiler and linker for this device
                (device.max_mem_alloc_size() >= min_mem_alloc_size) &&        // Device accepts large enough global memory allocations
                (device.global_mem_size() >= min_global_mem_size);            // Device has enough global memory
      }
   );

   // Create an OpenCL context on the device with some default parameters (see shared.hpp for more details)
   const auto context = Shared::build_default_context(selected_platform_and_device);

   // Allocate our input and output buffers
   std::cout << "Creating buffers..." << std::endl;
   const auto input_A_buffer = context.create_buffer(CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, vector_size);
   const auto input_B_buffer = context.create_buffer(CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, vector_size);
   const auto output_C_buffer = context.create_buffer(CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, vector_size);

   // Create a program object from the basic vector addition example
   std::cout << "Loading program..." << std::endl;
   auto program = context.create_program_with_source_file("kernels/vector_add.cl");

   // Start an asynchronous program build
   std::cout << "Starting to build program..." << std::endl;
   const auto build_event = program.build_with_event("-cl-mad-enable -cl-no-signed-zeros -cl-std=CL1.2 -cl-kernel-arg-info");

   // Create an out-of-order command queue for the device
   const auto command_queue = context.create_command_queue(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

   // Generate our input data and send it to the device
   std::cout << "Generating and sending data..." << std::endl;

   std::vector<cl_int> input_A(vector_length);
   for(size_t i = 0; i < vector_length; ++i) input_A[i] = i + 1;
   const auto write_A_event = command_queue.enqueued_write_buffer(static_cast<const void *>(&(input_A[0])), false, input_A_buffer, 0, vector_size, {});

   std::vector<cl_int> input_B(vector_length);
   for(size_t i = 0; i < vector_length; ++i) input_B[i] = vector_length - i;
   const auto write_B_event = command_queue.enqueued_write_buffer(static_cast<const void *>(&(input_B[0])), false, input_B_buffer, 0, vector_size, {});

   const auto all_write_events = command_queue.enqueued_marker_with_wait_list({write_A_event, write_B_event});

   // Once the program is built, create a kernel object associated to our vector addition routine
   std::cout << std::endl;
   std::cout << "Creating a kernel for vector addition..." << std::endl;
   const auto kernel = program.create_kernel("vector_add", build_event);

   // Set its arguments as appropriate
   kernel.set_buffer_argument(0, &input_A_buffer);
   kernel.set_buffer_argument(1, &input_B_buffer);
   kernel.set_buffer_argument(2, &output_C_buffer);

   // Execute the kernel
   std::cout << "Starting the kernel..." << std::endl;
   const auto exec_event = command_queue.enqueued_1d_range_kernel(kernel, vector_length, {all_write_events});

   // Once the kernel is done, synchronously read device output back into host memory
   std::cout << "Waiting for output..." << std::endl;
   std::vector<cl_int> output_C(vector_length);
   command_queue.read_buffer(output_C_buffer, 0, static_cast<void *>(&(output_C[0])), vector_size, {exec_event});

   // Verify the output
   std::cout << std::endl;
   for(const auto & output : output_C) {
      if(output != vector_length + 1) {
         std::cout << "Incorrect output !" << std::endl;
         std::abort();
      }
   }
   std::cout << "Vector addition was performed successfully !" << std::endl;

   return 0;
}
