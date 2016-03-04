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

#include "buffer.hpp"
#include "common.hpp"

namespace CLplusplus {

   Buffer & Buffer::operator=(const Buffer & source) {
      MemoryObject::operator=(source);
      return *this;
   }

   Buffer Buffer::create_sub_region(const cl_mem_flags flags, const cl_buffer_region & region) const {
      return raw_create_sub_buffer(flags, CL_BUFFER_CREATE_TYPE_REGION, static_cast<const void *>(&region));
   }

   Buffer Buffer::raw_create_sub_buffer(const cl_mem_flags flags, const cl_buffer_create_type buffer_create_type, const void * const buffer_create_info) const {
      cl_int error_code;
      const auto sub_buffer_id = clCreateSubBuffer(internal_id, flags, buffer_create_type, buffer_create_info, &error_code);
      throw_if_failed(error_code);
      return Buffer{sub_buffer_id, false};
   }

}
