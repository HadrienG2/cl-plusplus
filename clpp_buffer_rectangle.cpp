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

#include "CLplusplus/buffer.hpp"
#include "CLplusplus/device.hpp"
#include "CLplusplus/event.hpp"
#include "CLplusplus/platform.hpp"
#include "CLplusplus/version.hpp"

#include "shared.hpp"

// This program demonstrates rectangle-based buffer manipulation in CLplusplus
int main() {
   // Program parameters are defined here
   const cl_ulong buffer_width = 64;
   const cl_ulong buffer_height = 32;
   const cl_ulong buffer_size = buffer_width * buffer_height;

   // Minimal platform and device parameters are specified here
   const CLplusplus::Version target_version = CLplusplus::version_1p2;
   const cl_ulong min_mem_alloc_size = buffer_size;
   const cl_ulong min_local_mem_size = 16 * 1024; // TODO : Simplify this away

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

   // Create an OpenCL buffer, which we will use for rectangle-wise manipulations
   const auto buffer = context.create_buffer(CL_MEM_READ_WRITE, buffer_size);

   // Create an out-of-order command queue for the device
   const auto command_queue = context.create_command_queue(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

   // Prepare some pretty pattern to be written to the buffer
   cl_uchar input[buffer_size];
   for(size_t i = 0; i < buffer_size; ++i) input[i] = (255 - i) % 256;

   // Write the pattern to the buffer, as a rectangle
   std::cout << "Writing some pretty pattern to the buffer..." << std::endl;
   const size_t no_offset[] {0, 0};
   const size_t full_buffer_rect[] {buffer_width, buffer_height};
   const auto write_event = command_queue.enqueued_write_buffer_rect_2d(static_cast<const void *>(input), no_offset, buffer_width, false,
                                                                        buffer, no_offset, buffer_width,
                                                                        full_buffer_rect, {});

   // Transform the pattern in a checkerboard manner, getting from A  C to A  B
   //                                                              B  D    B  A
   std::cout << "Transforming it in a checkerboard manner..." << std::endl;
   const size_t half_width {buffer_width / 2};
   const size_t half_height {buffer_height / 2};
   const size_t lower_left[] {0, half_height};
   const size_t upper_right[] {half_width, 0};
   const size_t lower_right[] {half_width, half_height};
   const size_t buffer_quarter_rect[] {half_width, half_height};
   const auto copy_event_1 = command_queue.enqueued_copy_buffer_rect_2d(buffer, no_offset, buffer_width,
                                                                        buffer, lower_right, buffer_width,
                                                                        buffer_quarter_rect, {write_event});
   const auto copy_event_2 = command_queue.enqueued_copy_buffer_rect_2d(buffer, lower_left, buffer_width,
                                                                        buffer, upper_right, buffer_width,
                                                                        buffer_quarter_rect, {write_event});

   // Synchronously read back the result in an output buffer
   cl_uchar output[buffer_size];
   command_queue.read_buffer_rect_2d(buffer, no_offset, buffer_width,
                                     static_cast<void *>(output), no_offset, buffer_width,
                                     full_buffer_rect, {copy_event_1, copy_event_2});
   std::cout << "Result read back to host memory !" << std::endl << std::endl;

   // Check that the output matches our expectations
   for(size_t row = 0; row < buffer_height; ++row) {
      for(size_t col = 0; col < buffer_width; ++col) {
         const size_t output_i = row * buffer_width + col;

         const size_t input_col = (col < half_width) ? col : (col - half_width);
         const size_t input_row = (col < half_width) ? row : ((row < half_height) ? (row + half_height) : (row - half_height));
         const size_t input_i = input_row * buffer_width + input_col;

         if(output[output_i] != input[input_i]) {
            std::cout << "Data transformation failed !" << std::endl;
            std::abort();
         }
      }
   }
   std::cout << "Data was transformed successfully" << std::endl;

   return 0;
}
