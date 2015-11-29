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
#include "kernel.hpp"

namespace CLplusplus {

   Kernel::Kernel(const cl_kernel identifier, const bool increment_reference_count) :
      internal_id{identifier}
   {
      // Handle invalid kernel IDs
      if(internal_id == NULL) throw InvalidArgument();
      
      // Unless asked not to do so, increment the kernel object's reference count
      if(increment_reference_count) retain();
   }

   Kernel::Kernel(const Kernel & source) :
      internal_id{source.internal_id}
   {
      // Whenever a copy of a reference-counted kernel object is made, its reference count should be incremented
      retain();
   }

   Kernel & Kernel::operator=(const Kernel & source) {
      // Reference count considerations also apply to copy assignment operator
      internal_id = source.internal_id;
      retain();
      return *this;
   }

   std::string Kernel::raw_string_query(cl_kernel_info parameter_name) const {
      // Check how long the output string should be
      size_t output_string_length = raw_query_output_size(parameter_name);
      
      // Fetch the output string
      char output_string[output_string_length];
      raw_query(parameter_name, output_string_length, static_cast<void *>(output_string));

      // Return the result
      return std::string(output_string);
   }

   size_t Kernel::raw_query_output_size(const cl_kernel_info parameter_name) const {
      size_t result;
      raw_query(parameter_name, 0, nullptr, &result);
      return result;
   }

   void Kernel::raw_query(const cl_kernel_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size) const {
      throw_if_failed(clGetKernelInfo(internal_id, parameter_name, output_storage_size, output_storage, actual_output_size));
   }

   std::array<size_t, 3> Kernel::raw_work_group_size3_query(const Device & device, const cl_kernel_work_group_info parameter_name) const {
      std::array<size_t, 3> result;
      raw_work_group_query(device, parameter_name, 3 * sizeof(size_t), static_cast<void*>(&(result[0])), nullptr);
      return result;
   }

   size_t Kernel::raw_work_group_query_output_size(const Device & device, const cl_kernel_work_group_info parameter_name) const {
      size_t result;
      raw_work_group_query(device, parameter_name, 0, nullptr, &result);
      return result;
   }

   void Kernel::raw_work_group_query(const Device & device, const cl_kernel_work_group_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size) const {
      throw_if_failed(clGetKernelWorkGroupInfo(internal_id, device.raw_identifier(), parameter_name, output_storage_size, output_storage, actual_output_size));
   }

   std::string Kernel::raw_argument_string_query(const cl_uint arg_indx, const cl_kernel_arg_info parameter_name) const {
      // Check how long the output string should be
      size_t output_string_length = raw_query_output_size(parameter_name);
      
      // Fetch the output string
      char output_string[output_string_length];
      raw_argument_query(arg_indx, parameter_name, output_string_length, static_cast<void *>(output_string));

      // Return the result
      return std::string(output_string);
   }

   size_t Kernel::raw_argument_query_output_size(const cl_uint arg_indx, const cl_kernel_arg_info parameter_name) const {
      size_t result;
      raw_argument_query(arg_indx, parameter_name, 0, nullptr, &result);
      return result;
   }

   void Kernel::raw_argument_query(const cl_uint arg_indx, const cl_kernel_arg_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size) const {
      throw_if_failed(clGetKernelArgInfo(internal_id, arg_indx, parameter_name, output_storage_size, output_storage, actual_output_size));
   }

   void Kernel::set_buffer_argument(const cl_uint arg_index, const Buffer * arg_value) const {
      if(arg_value) {
         const auto buffer_id = arg_value->raw_identifier();
         raw_set_argument(arg_index, sizeof(cl_mem), static_cast<const void *>(&buffer_id));
      } else {
         raw_set_argument(arg_index, sizeof(cl_mem), nullptr);
      }
   }

   void Kernel::raw_set_argument(const cl_uint arg_index, const size_t arg_size, const void * const arg_value) const {
      throw_if_failed(clSetKernelArg(internal_id, arg_index, arg_size, arg_value));
   }

   void Kernel::retain() const {
      throw_if_failed(clRetainKernel(internal_id));
   }

   void Kernel::release() {
      throw_if_failed(clReleaseKernel(internal_id));
   }

}
