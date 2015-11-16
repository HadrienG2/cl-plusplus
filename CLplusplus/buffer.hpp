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

#ifndef INCLUDE_CL_PLUSPLUS_BUFFER
#define INCLUDE_CL_PLUSPLUS_BUFFER

#include <CL/cl.h>

#include "memory_object.hpp"

// This code unit defines high-level ways to manage OpenCL buffers
namespace CLplusplus {

   class Buffer : MemoryObject {
      public:
         // Buffer objects can be created from a valid OpenCL identifier
         Buffer(const cl_mem identifier, const bool increment_reference_count) : MemoryObject{identifier, increment_reference_count} {}

         // Buffer objects are reference counted using the following functions
         Buffer(const Buffer & source) : MemoryObject{source} {}
         Buffer & operator=(const Buffer & source);

         // If is possible to create sub-buffers from OpenCL buffers. For wrapper-supported sub-buffer types, this is clean and easy.
         Buffer create_sub_region(const cl_mem_flags flags, const cl_buffer_region & region) const;

         // And for wrapper-unsupported sub-buffer types, we revert to the raw OpenCL syntax for sub-buffer creation.
         Buffer raw_create_sub_buffer(const cl_mem_flags flags, const cl_buffer_create_type buffer_create_type, const void * const buffer_create_info) const;
   };

}

#endif
