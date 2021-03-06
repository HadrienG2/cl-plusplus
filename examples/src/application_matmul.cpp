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

#include <array>
#include <cmath>
#include <iostream>

#include <CL/cl.h>

#include "CLplusplus/device.hpp"
#include "CLplusplus/kernel.hpp"
#include "CLplusplus/platform.hpp"
#include "CLplusplus/program.hpp"
#include "CLplusplus/version.hpp"

#include "shared.hpp"

// This program performs double matrix multiplication, evaluating the efficiency of transposing the right-hand-side first as a performance optimization
int main() {
   // === PROGRAM PARAMETERS ===

   // In this example, we will be multiplying two square double matrices of a certain size
   const unsigned int matrix_side_length = 4096;
   const std::array<size_t, 2> global_work_size = {matrix_side_length, matrix_side_length};
   const size_t matrix_size = global_work_size[0] * global_work_size[1] * sizeof(cl_double);

   const unsigned int workgroup_side_length = 32;
   const std::array<size_t, 2> local_work_size = {workgroup_side_length, workgroup_side_length};
   const size_t transpose_local_buf_size = local_work_size[0] * local_work_size[1] * sizeof(cl_double);

   // Minimal platform and device parameters are specified here
   const CLplusplus::Version target_version = CLplusplus::version_1p2;
   const cl_ulong min_mem_alloc_size = matrix_size;
   const cl_ulong min_global_mem_size = 4 * matrix_size;
   const cl_ulong min_local_mem_size = transpose_local_buf_size;

   // === INITIALIZATION ===

   // Have the user select a suitable device, according to some criteria (see shared.hpp for more details)
   const auto selected_platform_and_device = Shared::select_device(
      [&](const CLplusplus::Platform & platform) -> bool {
         return (platform.version() >= target_version);                       // Platform OpenCL version is recent enough
      },
      [&](const CLplusplus::Device & device) -> bool {
         if(device.version() < target_version) return false;                  // OpenCL platforms may support older-generation devices, which we need to eliminate

         const auto queue_properties = device.queue_properties();
         const bool device_supports_ooe_execution = queue_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
         const bool device_supports_profiling = queue_properties & CL_QUEUE_PROFILING_ENABLE;

         const auto max_work_item_size = device.max_work_item_sizes();
         const auto device_supports_launch_config = (device.max_work_item_dimensions() >= 2) && (max_work_item_size[0] >= local_work_size[0]) && (max_work_item_size[1] >= local_work_size[1]);

         const auto device_double_config = device.double_fp_config();

         return device.available() &&                                         // Device is available for compute purposes
                device.endian_little() &&                                     // Device is little-endian
                (device.execution_capabilities() & CL_EXEC_KERNEL) &&         // Device can execute OpenCL kernels

                device_supports_ooe_execution &&                              // Device can execute OpenCL commands out of order
                device_supports_profiling &&                                  // Device supports OpenCL command profiling

                device.compiler_available() && device.linker_available() &&   // Implementation has an OpenCL C compiler and linker for this device

                (device.max_mem_alloc_size() >= min_mem_alloc_size) &&        // Device accepts large enough global memory allocations
                (device.global_mem_size() >= min_global_mem_size) &&          // Device has enough global memory

                (device.local_mem_type() == CL_LOCAL) &&                      // Device has local memory support, with dedicated storage
                (device.local_mem_size() >= min_local_mem_size) &&            // Device has a large enough local memory

                device_supports_launch_config &&                              // Device supports our desired kernel launch configuration

                (device_double_config != 0) &&                                // Doubles are supported
                ((device_double_config & CL_FP_SOFT_FLOAT) == 0);             // Doubles are not emulated in software
      }
   );

   // Create an OpenCL context on the device with some default parameters (see shared.hpp for more details)
   const auto context = Shared::build_default_context(selected_platform_and_device);

   // Allocate our input and output buffers
   std::cout << "Creating buffers..." << std::endl;
   const auto input_matrix_A_buf = context.create_buffer(CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, matrix_size);
   const auto input_matrix_B_buf = context.create_buffer(CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, matrix_size);
   const auto transposed_B_buf = context.create_buffer(CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, matrix_size);
   const auto output_matrix_buf = context.create_buffer(CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, matrix_size);

   // Create programs object from the matrix transpose and matrix multiplication examples
   std::cout << "Loading programs..." << std::endl;
   auto transpose_program = context.create_program_with_source_file("kernels/matrix_transpose_double.cl");
   auto matmul_program = context.create_program_with_source_file("kernels/matrix_multiply_double.cl");

   // Start asynchronous program builds
   std::cout << "Starting to build program..." << std::endl;
   const auto matmul_build_event = matmul_program.build_with_event("-cl-mad-enable -cl-no-signed-zeros -cl-std=CL1.2 -cl-kernel-arg-info");
   const auto transpose_build_event = transpose_program.build_with_event("-cl-mad-enable -cl-no-signed-zeros -cl-std=CL1.2 -cl-kernel-arg-info");

   // Create an out-of-order command queue for the device
   const auto command_queue = context.create_command_queue(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE);

   // Generate our input data and send it to the device
   std::cout << "Generating and sending data..." << std::endl;
   const auto matrix_length = matrix_side_length * matrix_side_length;

   std::vector<cl_double> input_matrix_A(matrix_length);
   for(size_t i = 0; i < matrix_length; ++i) input_matrix_A[i] = i + 1;
   const auto write_A_event = command_queue.enqueued_write_buffer(static_cast<const void *>(&(input_matrix_A[0])), false, input_matrix_A_buf, 0, matrix_size, {});

   std::vector<cl_double> input_matrix_B(matrix_length);
   for(size_t i = 0; i < matrix_length; ++i) input_matrix_B[i] = 3 * i + 2;
   const auto write_B_event = command_queue.enqueued_write_buffer(static_cast<const void *>(&(input_matrix_B[0])), false, input_matrix_B_buf, 0, matrix_size, {});

   // === NAIVE MATRIX MULTIPLICATION ===

   // Create a kernel object associated to the naive matrix multiplication routine
   std::cout << std::endl;
   std::cout << "Creating a kernel for naive matrix multiplication" << std::endl;
   const auto kernel_matmul_naive = matmul_program.create_kernel("double_matmul_naive", matmul_build_event);

   // Set its arguments as appropriate
   kernel_matmul_naive.set_buffer_argument(0, &input_matrix_A_buf);
   kernel_matmul_naive.set_buffer_argument(1, &input_matrix_B_buf);
   kernel_matmul_naive.set_buffer_argument(2, &output_matrix_buf);

   // Begin kernel execution
   std::cout << "Starting the kernel..." << std::endl;
   const auto exec_event_matmul_naive = command_queue.enqueued_2d_range_kernel(kernel_matmul_naive, global_work_size, local_work_size, {write_A_event, write_B_event});

   // Once the kernel is done, synchronously read device output back into host memory
   std::cout << "Waiting for output..." << std::endl;
   std::vector<cl_double> output_matrix_naive(matrix_length);
   command_queue.read_buffer(output_matrix_buf, 0, static_cast<void *>(&(output_matrix_naive[0])), matrix_size, {exec_event_matmul_naive});

   // Tell the profiled performance of this work
   std::cout << "The naive matrix multiplication kernel executed in " << (exec_event_matmul_naive.end_time_ns() - exec_event_matmul_naive.start_time_ns()) / 1000000 << " milliseconds" << std::endl;

   // === TRANSPOSE-BASED OPTIMIZED IMPLEMENTATION ===

   // Create a kernel object associated to the local memory matrix transpose routine
   std::cout << std::endl;
   std::cout << "Creating a kernel for local memory matrix transposition..." << std::endl;
   const auto kernel_transpose_local = transpose_program.create_kernel("double_transpose_local", transpose_build_event);

   // Set its arguments as appropriate
   kernel_transpose_local.set_buffer_argument(0, &input_matrix_B_buf);
   kernel_transpose_local.set_local_argument(1, transpose_local_buf_size);   
   kernel_transpose_local.set_buffer_argument(2, &transposed_B_buf);

   // Begin local mem transpose execution
   std::cout << "Starting the kernel..." << std::endl;
   const auto exec_event_transpose_local = command_queue.enqueued_2d_range_kernel(kernel_transpose_local, global_work_size, local_work_size, {write_B_event});

   // While the transpose kernel is executing, create a kernel object associated to the optimized matrix multiplication routine
   std::cout << "Creating a kernel for the optimized matrix multiplication..." << std::endl;
   const auto kernel_matmul_transpose = matmul_program.create_kernel("double_matmul_transpose", matmul_build_event);

   // Set its arguments as appropriate
   kernel_matmul_transpose.set_buffer_argument(0, &input_matrix_A_buf);
   kernel_matmul_transpose.set_buffer_argument(1, &transposed_B_buf);
   kernel_matmul_transpose.set_buffer_argument(2, &output_matrix_buf);

   // Queue this kernel for execution after B transposition is completed
   std::cout << "Scheduling it to run after the transpose..." << std::endl;
   const auto exec_event_matmul_transpose = command_queue.enqueued_2d_range_kernel(kernel_matmul_transpose, global_work_size, local_work_size, {write_A_event, exec_event_transpose_local});

   // Once all kernels are done, synchronously read device output back into host memory
   std::cout << "Waiting for output..." << std::endl;
   std::vector<cl_double> output_matrix_optimized(matrix_length);
   command_queue.read_buffer(output_matrix_buf, 0, static_cast<void *>(&(output_matrix_optimized[0])), matrix_size, {exec_event_matmul_transpose});

   // Tell the profiled performance of this second output
   std::cout << "The optimized kernel executed in " << (exec_event_matmul_transpose.end_time_ns() - exec_event_matmul_transpose.start_time_ns()) / 1000000 << " milliseconds" << std::endl;

   // === RESULT COMPARISON ===

   // Check that the naive and optimized matrix multiplications produce the same result (we will assume the naive one is right)
   std::cout << std::endl;
   for(size_t i = 0; i < matrix_length; ++i) {
      const auto absolute_difference = fabs(output_matrix_optimized[i] - output_matrix_naive[i]);
      if(absolute_difference > 0.001 * fabs(output_matrix_naive[i])) {
         std::cout << "Matrix product output mismatch at index " << i << "!" << std::endl;
         std::cout << "Optimized[i] = " << output_matrix_optimized[i] << " vs naive[i] = " << output_matrix_naive[i] << std::endl;
         std::cout << "Absolute difference is " << absolute_difference << " = " << absolute_difference/fabs(output_matrix_naive[i]) << " times naive value" << std::endl;
         std::abort();
      }
   }
   std::cout << "Naive and optimized matrix multiplication agree!" << std::endl;

   return 0;
}
