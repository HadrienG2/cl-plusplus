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
#include "program.hpp"

namespace CLplusplus {

   Program::Program(const cl_program identifier, const bool increment_reference_count) :
      internal_id{identifier}
   {
      // Handle invalid program IDs
      if(internal_id == NULL) throw InvalidArgument();
      
      // Unless asked not to do so, increment the program object's reference count
      if(increment_reference_count) retain();
   }

   Program::Program(const Program & source) :
      internal_id{source.internal_id}
   {
      // Whenever a copy of a reference-counted memory object is made, its reference count should be incremented
      retain();
   }

   Program & Program::operator=(const Program & source) {
      // Reference count considerations also apply to copy assignment operator
      internal_id = source.internal_id;
      retain();
      return *this;
   }

   size_t Program::raw_query_output_size(const cl_program_info parameter_name) const {
      size_t result;
      raw_query(parameter_name, 0, nullptr, &result);
      return result;
   }

   void Program::raw_query(const cl_program_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size) const {
      throw_if_failed(clGetProgramInfo(internal_id, parameter_name, output_storage_size, output_storage, actual_output_size));
   }

   void Program::retain() const {
      throw_if_failed(clRetainProgram(internal_id));
   }

   void Program::release() {
      throw_if_failed(clReleaseProgram(internal_id));
   }

}
