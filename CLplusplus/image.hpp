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

#ifndef INCLUDE_CL_PLUSPLUS_IMAGE
#define INCLUDE_CL_PLUSPLUS_IMAGE

#include <CL/cl.h>

#include "memory_object.hpp"

// This code unit defines high-level ways to manage OpenCL images
namespace CLplusplus {

   class Image : public MemoryObject {
      public:
         // === BASIC CLASS LIFECYCLE ===

         // Image objects can be created from a valid OpenCL identifier
         Image(const cl_mem identifier, const bool increment_reference_count) : MemoryObject{identifier, increment_reference_count} {}

         // Image objects are reference counted using the following functions
         Image(const Image & source) : MemoryObject{source} {}
         Image & operator=(const Image & source);

         // === IMAGE-SPECIFIC PROPERTIES ===

         // Image properties which are supported by the wrapper are directly accessible in a convenient, high-level fashion
         cl_image_format image_format() const { return raw_image_value_query<cl_image_format>(CL_IMAGE_FORMAT); }
         size_t image_element_size() const { return raw_image_size_query(CL_IMAGE_ELEMENT_SIZE); }
         size_t image_row_pitch() const { return raw_image_size_query(CL_IMAGE_ROW_PITCH); }
         size_t image_slice_pitch() const { return raw_image_size_query(CL_IMAGE_SLICE_PITCH); }
         size_t image_width() const { return raw_image_size_query(CL_IMAGE_WIDTH); }
         size_t image_height() const { return raw_image_size_query(CL_IMAGE_HEIGHT); }
         size_t image_depth() const { return raw_image_size_query(CL_IMAGE_DEPTH); }
         size_t image_array_size() const { return raw_image_size_query(CL_IMAGE_ARRAY_SIZE); }
         cl_uint image_num_mip_levels() const { return raw_image_uint_query(CL_IMAGE_NUM_MIP_LEVELS); }
         cl_uint image_num_samples() const { return raw_image_uint_query(CL_IMAGE_NUM_SAMPLES); }

         // Unsupported property values may be queried in a lower-level way
         cl_mem raw_image_buffer_id() const { return raw_value_query<cl_mem>(CL_IMAGE_BUFFER); }  // WARNING : Beware that trying to use this identifier in a Buffer can lead to callback memory leaks.

         // And fully unsupported image properties can be queried in a nearly pure OpenCL way, with some common-case usability optimizations
         size_t raw_image_size_query(const cl_image_info parameter_name) const { return raw_image_value_query<size_t>(parameter_name); }
         cl_uint raw_image_uint_query(const cl_image_info parameter_name) const { return raw_image_value_query<cl_uint>(parameter_name); }

         template<typename ValueType> ValueType raw_image_value_query(const cl_image_info parameter_name) const {
            ValueType result;
            raw_image_query(parameter_name, sizeof(ValueType), &result);
            return result;
         }

         size_t raw_image_query_output_size(const cl_image_info parameter_name) const;
         void raw_image_query(const cl_image_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size = nullptr) const;
   };

}

#endif
