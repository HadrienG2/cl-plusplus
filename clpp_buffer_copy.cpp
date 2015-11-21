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

// This program demonstrates buffer copies in CLplusplus
int main() {
   // Program parameters are defined here
   const cl_ulong buffer_size = 4096;

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

   // Create two OpenCL buffers.
   // One will be written to by the host, and thus serve as a copy input.
   // The other will serve as a copy output, and be read and checked by the host.
   const auto input_buffer = context.create_buffer(CL_MEM_READ_WRITE, buffer_size);
   const auto output_buffer = context.create_buffer(CL_MEM_READ_WRITE, buffer_size);

   // Create an out-of-order command queue for the device
   const auto command_queue = context.create_command_queue(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

   // Write some pretty pattern to the input buffer
   cl_uchar input[buffer_size];
   for(size_t i = 0; i < buffer_size; ++i) input[i] = (255 - i) % 256;
   const auto write_event = command_queue.enqueued_write_buffer(static_cast<const void *>(input), false, input_buffer, 0, buffer_size, {});
   std::cout << "Writing some pretty pattern to the input buffer..." << std::endl;

   // Copy the pattern from the input buffer to the output buffer
   const auto copy_event = command_queue.enqueued_copy_buffer(input_buffer, 0, output_buffer, 0, buffer_size, {write_event});
   std::cout << "Copying it to the output buffer..." << std::endl;

   // Read back the result in an output buffer
   cl_uchar output[buffer_size];
   const auto read_event = command_queue.enqueued_read_buffer(output_buffer, 0, static_cast<void *>(output), buffer_size, {copy_event});
   std::cout << "Reading it back to host memory..." << std::endl << std::endl;

   // Wait for all the last read to finish
   CLplusplus::wait_for_events({read_event});

   // Check that the output matches the input
   for(size_t i = 0; i < buffer_size; ++i) {
      if(input[i] != output[i]) {
         std::cout << "Data transmission failed !" << std::endl;
         std::abort();
      }
   }
   std::cout << "Data was transmitted successfully" << std::endl;

   return 0;
}
