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

// This program demonstrates buffer filling and mapping in CLplusplus
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

   // Start to fill the buffer with a constant pattern
   std::cout << "Filling buffer..." << std::endl;
   const cl_uchar pattern = 0x42;
   const auto fill_event = command_queue.enqueued_fill_buffer(pattern, buffer, 0, buffer_size, {});

   // Ask the buffer to be mapped to host memory once this is done, and synchronize on this event
   cl_uchar * const mapped_buffer = static_cast<cl_uchar *>(command_queue.map_buffer(buffer, 0, buffer_size, CL_MAP_READ, {fill_event}));
   std::cout << "Buffer mapped !" << std::endl;

   // Check that the buffer was filled as expected
   bool fill_error = false;
   for(unsigned int i = 0; i < buffer_size; ++i) {
      if(mapped_buffer[i] != 0x42) {
         std::cout << "Buffer fill error !" << std::endl;
         fill_error = true;
         break;
      }
   }
   if(!fill_error) std::cout << "Buffer was filled up successfully" << std::endl;

   // Unmap the buffer and terminate
   command_queue.enqueue_unmap_mem_object(buffer, mapped_buffer, {});
   command_queue.finish();
   std::cout << "Buffer unmapped !" << std::endl;

   return 0;
}
