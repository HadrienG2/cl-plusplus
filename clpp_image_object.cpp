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
#include "CLplusplus/image.hpp"
#include "CLplusplus/platform.hpp"
#include "CLplusplus/version.hpp"

#include "shared.hpp"

// This program demonstrates image creation and basic queries in CLplusplus
int main() {
   // Program parameters are defined here
   const cl_ulong image_length = 4096;
   const size_t image_size = image_length * 4;  

   // Minimal platform and device parameters are specified here
   const CLplusplus::Version target_version = CLplusplus::version_1p2;
   const cl_ulong min_mem_alloc_size = image_size;
   const cl_ulong min_global_mem_size = image_size;

   // Have the user select a suitable device, according to some criteria (see shared.hpp for more details)
   const auto selected_platform_and_device = Shared::select_device(
      [&](const CLplusplus::Platform & platform) -> bool {
         return (platform.version() >= target_version);                       // Platform OpenCL version is recent enough
      },
      [&](const CLplusplus::Device & device) -> bool {
         if(device.version() < target_version) return false;                  // OpenCL platforms may support older-generation devices, which we need to eliminate
         return device.available() &&                                         // Device is available for compute purposes
                (device.max_mem_alloc_size() >= min_mem_alloc_size) &&        // Device accepts large enough global memory allocations
                (device.global_mem_size() >= min_global_mem_size) &&          // Device has enough global memory

                device.image_support() &&                                     // Device supports images
                (device.image2d_max_width() > image_length);                  // Device accepts large enough image widths
      }
   );

   // Create an OpenCL context on the device with some default parameters (see shared.hpp for more details)
   const auto context = Shared::build_default_context(selected_platform_and_device);

   // Create a small OpenCL image
   cl_image_format image_format;
   image_format.image_channel_order = CL_RGBA;
   image_format.image_channel_data_type = CL_UNORM_INT8;
   cl_image_desc image_desc;
   image_desc.image_type = CL_MEM_OBJECT_IMAGE1D;
   image_desc.image_width = image_length;
   image_desc.image_row_pitch = 0;
   image_desc.image_slice_pitch = 0;
   image_desc.num_mip_levels = 0;
   image_desc.num_samples = 0;
   image_desc.buffer = NULL;
   const auto image = context.create_image(CL_MEM_READ_WRITE, image_format, image_desc);

   // Display the memory object properties of our image
   std::cout << "Our newly created image objet is ";
   switch(image.type()) {
      case CL_MEM_OBJECT_IMAGE1D:
         std::cout << "a 1D image";
         break;
      case CL_MEM_OBJECT_IMAGE1D_BUFFER:
         std::cout << "a buffer-based 1D image";
         break;
      case CL_MEM_OBJECT_IMAGE1D_ARRAY:
         std::cout << "a 1D image array";
         break;
      case CL_MEM_OBJECT_IMAGE2D:
         std::cout << "a 2D image";
         break;
      case CL_MEM_OBJECT_IMAGE2D_ARRAY:
         std::cout << "a 2D image array";
         break;
      case CL_MEM_OBJECT_IMAGE3D:
         std::cout << "a 3D image";
         break;
      default:
         std::cout << "something strange and unforeseen";
   }
   std::cout << std::endl;

   const auto flags = image.flags();
   std::cout << "Its flags are ";
   if(flags & CL_MEM_WRITE_ONLY) std::cout << "WRITE_ONLY ";
   else if(flags & CL_MEM_READ_ONLY) std::cout << "READ_ONLY ";
   else std::cout << "READ_WRITE ";
   if(flags & CL_MEM_USE_HOST_PTR) std::cout << "USE_HOST_PTR ";
   if(flags & CL_MEM_ALLOC_HOST_PTR) std::cout << "ALLOC_HOST_PTR ";
   if(flags & CL_MEM_COPY_HOST_PTR) std::cout << "COPY_HOST_PTR ";
   if(flags & CL_MEM_HOST_WRITE_ONLY) std::cout << "HOST_WRITE_ONLY";
   if(flags & CL_MEM_HOST_READ_ONLY) std::cout << "HOST_READ_ONLY";
   if(flags & CL_MEM_HOST_NO_ACCESS) std::cout << "HOST_NO_ACCESS";
   std::cout << std::endl;

   std::cout << "It is " << image.size() << " bytes long" << std::endl;

   const auto host_ptr = image.host_ptr();
   std::cout << "Its host pointer points to address ";
   if(host_ptr == nullptr) {
      std::cout << "NULL";
   } else {
      std::cout << (size_t)(void *)host_ptr;
   }
   std::cout << std::endl;

   std::cout << "Our image is currently being mapped " << image.map_count() << " times" << std::endl;

   std::cout << "Our image ";
   if(image.raw_context_id() == context.raw_identifier()) {
      std::cout << "identifies with the right context";
   } else {
      std::cout << "seems to deny its context of origin, which is problematic";
   }
   std::cout << std::endl;

   // Display image-specific properties
   std::cout << std::endl;

   const auto actual_format = image.image_format();
   std::cout << "Out image ";
   if((actual_format.image_channel_order == image_format.image_channel_order) &&
      (actual_format.image_channel_data_type == image_format.image_channel_data_type)) {
      std::cout << "has";
   } else {
      std::cout << "does not have";
   }
   std::cout << " the format we requested" << std::endl;

   std::cout << "Each image element weights " << image.image_element_size() << " bytes" << std::endl;

   std::cout << "The image row pitch is " << image.image_row_pitch() << " bytes" << std::endl;
   std::cout << "The image slice pitch is " << image.image_slice_pitch() << " bytes" << std::endl;

   std::cout << "The image dimensions are (" << image.image_width() << ", " << image.image_height() << ", " << image.image_depth() << ")" << std::endl;

   const auto array_size = image.image_array_size();
   std::cout << "The image object is ";
   if(array_size == 0) {
      std::cout << "not an array";
   } else {
      std::cout << "an array of " << array_size << " items";
   }
   std::cout << std::endl;

   std::cout << "The image has " << image.image_num_mip_levels() << " mip levels" << std::endl;

   std::cout << "The image has " << image.image_num_samples() << " samples per pixels" << std::endl;

   return 0;
}
