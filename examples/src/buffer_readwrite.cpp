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
#include "CLplusplus/platform.hpp"
#include "CLplusplus/version.hpp"

#include "shared.hpp"

// This program demonstrates basic buffer handling in CLplusplus
int main() {
   // Program parameters are defined here
   const cl_ulong buffer_size = 4096;

   // Minimal platform and device parameters are specified here
   const CLplusplus::Version target_version = CLplusplus::version_1p2;
   const cl_ulong min_mem_alloc_size = buffer_size;
   const cl_ulong min_global_mem_size = buffer_size;

   // Have the user select a suitable device, according to some criteria (see shared.hpp for more details)
   const auto selected_platform_and_device = Shared::select_device(
      [&](const CLplusplus::Platform & platform) -> bool {
         return (platform.version() >= target_version);                       // Platform OpenCL version is recent enough
      },
      [&](const CLplusplus::Device & device) -> bool {
         if(device.version() < target_version) return false;                  // OpenCL platforms may support older-generation devices, which we need to eliminate
         const bool device_supports_ooe_execution = device.queue_properties() & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
         return device.available() &&                                         // Device is available for compute purposes
                device_supports_ooe_execution &&                              // Device can execute OpenCL commands out of order
                (device.max_mem_alloc_size() >= min_mem_alloc_size) &&        // Device accepts large enough global memory allocations
                (device.global_mem_size() >= min_global_mem_size);            // Device has enough global memory
      }
   );

   // Create an OpenCL context on the device with some default parameters (see shared.hpp for more details)
   const auto context = Shared::build_default_context(selected_platform_and_device);

   // Create a small OpenCL buffer
   const auto buffer = context.create_buffer(CL_MEM_READ_WRITE, buffer_size);

   // Create an out-of-order command queue for the device
   const auto command_queue = context.create_command_queue(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);

   // Prepare some input data to be sent to the buffer
   cl_uchar input[buffer_size];
   for(size_t i = 0; i < buffer_size; ++i) input[i] = (255 - i) % 256;

   // Schedule to send the input data to the buffer. Here, we use the "enqueued" syntax, which gives us access to the associated OpenCL event.
   auto send_event = command_queue.enqueued_write_buffer(static_cast<const void *>(input), false, buffer, 0, buffer_size, {});
   send_event.set_callback(CL_COMPLETE,
      [](cl_event unused, cl_int command_status) {
         if(command_status == CL_COMPLETE) {
            std::cout << "Input data has been successfully written to the buffer" << std::endl;
         } else {
            std::cout << "An error occurred while sending input data" << std::endl;
         }
      }
   );

   // Schedule to get the data back. Note that here, we use a different syntax ("enqueue") which does not give us access to the read event.
   // This may be handy for quick coding, but the lack of error handling makes use of this variant somewhat unadvisable in production code.
   cl_uchar output[buffer_size];
   command_queue.enqueue_read_buffer(buffer, 0, static_cast<void *>(output), buffer_size, {send_event});

   // Wait for all pending commands to finish
   command_queue.finish();
   std::cout << "Output data should now be fetched back from the buffer" << std::endl;

   // Compare the input and output buffers' contents
   for(size_t i = 0; i < buffer_size; ++i) {
      if(input[i] != output[i]) {
         std::cout << "Data transmission failed !" << std::endl;
         std::abort();
      }
   }
   std::cout << "Data was transmitted successfully" << std::endl;

   return 0;
}
