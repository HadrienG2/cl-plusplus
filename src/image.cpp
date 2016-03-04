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

#include "common.hpp"
#include "image.hpp"

namespace CLplusplus {

   Image & Image::operator=(const Image & source) {
      MemoryObject::operator=(source);
      return *this;
   }

   size_t Image::raw_image_query_output_size(const cl_image_info parameter_name) const {
      size_t result;
      raw_image_query(parameter_name, 0, nullptr, &result);
      return result;
   }

   void Image::raw_image_query(const cl_image_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size) const {
      throw_if_failed(clGetImageInfo(internal_id, parameter_name, output_storage_size, output_storage, actual_output_size));
   }

}
