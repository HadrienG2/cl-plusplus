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

   std::vector<Device> Program::devices() const {
      // Check how many devices are associated to the program and allocate storage accordingly
      const auto device_amount = num_devices();
      cl_device_id raw_device_ids[device_amount];

      // Request the device ID list
      raw_query(CL_PROGRAM_DEVICES, device_amount * sizeof(cl_device_id), static_cast<void *>(raw_device_ids));

      // Convert it into high-level output
      std::vector<Device> result;
      for(unsigned int i = 0; i < device_amount; ++i) result.emplace_back(Device{raw_device_ids[i], true});
      return result;
   }

   std::vector<size_t> Program::binary_sizes() const {
      // Check how many devices are associated to the program and allocate storage accordingly
      const auto device_amount = num_devices();
      std::vector<size_t> result(device_amount);

      // Request the binary size list
      raw_query(CL_PROGRAM_BINARY_SIZES, device_amount * sizeof(size_t), static_cast<void *>(&(result[0])));

      // Return the result
      return result;
   }

   std::vector<ProgramBinary> Program::binaries() const {
      // Determine how large our binary storage should be and allocate it
      const auto required_storage = binary_sizes();
      const auto num_devices = required_storage.size();
      std::vector<ProgramBinary> result(num_devices);
      for(size_t i = 0; i < num_devices; ++i) result[i].resize(required_storage[i]);

      // Prepare a view of our binary storage which is compatible with OpenCL conventions
      unsigned char * raw_storage_view[num_devices];
      for(size_t i = 0; i < num_devices; ++i) raw_storage_view[i] = &(result[i][0]);

      // Ask OpenCL to provide us with the binaries and return them
      raw_get_binaries(num_devices, raw_storage_view);
      return result;
   }

   void Program::raw_get_binaries(const size_t device_amount, unsigned char * dest_storage[]) const {
      raw_query(CL_PROGRAM_BINARIES, device_amount * sizeof(unsigned char *), static_cast<void *>(dest_storage));
   }

   std::string Program::raw_string_query(cl_program_info parameter_name) const {
      // Check how long the output string should be
      size_t output_string_length = raw_query_output_size(parameter_name);
      
      // Fetch the output string
      char output_string[output_string_length];
      raw_query(parameter_name, output_string_length, static_cast<void *>(output_string));

      // Return the result
      return std::string(output_string);
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
