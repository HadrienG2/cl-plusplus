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

#include <functional>

#include "common.hpp"
#include "context.hpp"

namespace CLplusplus {

   Context::Context(const cl_context identifier) : internal_id{identifier} {
      // Handle invalid context IDs
      if(internal_id == NULL) throw InvalidArgument();
   }

   Context::Context(ContextProperties & properties, const Device & device, const ContextCallback & callback) :
      internal_callback_ptr{callback ? (new ContextCallback{callback}) : nullptr}
   {
      create_context(properties, 1, &device);
   }

   Context::Context(ContextProperties & properties, const Device & device, const ContextCallbackWithUserData & callback, void * const user_data) {
      using namespace std::placeholders;
      internal_callback_ptr = new ContextCallback{std::bind(callback, _1, _2, _3, user_data)};
      create_context(properties, 1, &device);
   }

   Context::Context(ContextProperties & properties, const std::vector<Device> & devices, const ContextCallback & callback) :
      internal_callback_ptr{callback ? (new ContextCallback{callback}) : nullptr}
   {
      create_context(properties, devices.size(), &devices[0]);
   }

   Context::Context(ContextProperties & properties, const std::vector<Device> & devices, const ContextCallbackWithUserData & callback, void * const user_data) {
      using namespace std::placeholders;
      internal_callback_ptr = new ContextCallback{std::bind(callback, _1, _2, _3, user_data)};
      create_context(properties, devices.size(), &devices[0]);
   }

   Context::Context(ContextProperties & properties, const cl_device_type device_type, const ContextCallback & callback) :
      internal_callback_ptr{callback ? (new ContextCallback{callback}) : nullptr}
   {
      create_context_from_type(properties, device_type);
   }

   Context::Context(ContextProperties & properties, const cl_device_type device_type, const ContextCallbackWithUserData & callback, void * const user_data) {
      using namespace std::placeholders;
      internal_callback_ptr = new ContextCallback{std::bind(callback, _1, _2, _3, user_data)};
      create_context_from_type(properties, device_type);
   }

   Context::Context(const Context & source) {
      // Whenever a copy of a reference-counted context is made, its reference count should be incremented
      copy_internal_data(source);
      retain_context();
   }

   Context::~Context() {
      // Decrement context reference count, possibly causing context liberation
      release_context();
   }

   Context & Context::operator=(const Context & source) {
      // Reference count considerations also apply to copy assignment operator
      copy_internal_data(source);
      retain_context();
      return *this;
   }

   std::vector<Device> Context::devices() const {
      // Check how many devices are available on the platform and allocate storage accordingly
      const auto device_amount = num_devices();
      cl_device_id opencl_devices[device_amount];

      // Request the context to provide a device ID list
      raw_query(CL_CONTEXT_DEVICES, device_amount * sizeof(cl_device_id), (void *)opencl_devices);

      // Convert it into high-level output
      std::vector<Device> result;
      for(unsigned int i = 0; i < device_amount; ++i) result.emplace_back(Device{opencl_devices[i]});
      return result;
   }

   ContextProperties Context::properties() const {
      // Check the length of the null-terminated OpenCL property list and allocate storage accordingly
      const auto property_list_length = raw_query_output_size(CL_CONTEXT_PROPERTIES) / sizeof(cl_context_properties);
      cl_context_properties opencl_properties[property_list_length];

      // Request the context to provide the property list
      raw_query(CL_CONTEXT_PROPERTIES, property_list_length * sizeof(cl_context_properties), (void *)opencl_properties);

      // Convert it into high-level output
      return ContextProperties{opencl_properties};
   }

   cl_uint Context::raw_uint_query(const cl_context_info parameter_name) const {
      cl_uint result;
      raw_query(parameter_name, sizeof(cl_uint), &result);
      return result;
   }

   size_t Context::raw_query_output_size(cl_context_info parameter_name) const {
      size_t result;
      raw_query(parameter_name, 0, nullptr, &result);
      return result;
   }

   void Context::raw_query(cl_context_info parameter_name, size_t output_storage_size, void * output_storage, size_t * actual_output_size) const {
      throw_if_failed(clGetContextInfo(internal_id, parameter_name, output_storage_size, output_storage, actual_output_size));
   }

   void CL_CALLBACK Context::raw_callback(const char * errinfo, const void * private_info, size_t cb, void * actual_callback_ptr) {
      const auto actual_callback = static_cast<const ContextCallback *>(actual_callback_ptr);
      (*actual_callback)(std::string{errinfo}, private_info, cb);
   }

   void Context::create_context(ContextProperties & properties, const cl_uint device_count, const Device * devices) {
      // Extract the internal identifiers of our devices
      cl_device_id device_ids[device_count];
      for(unsigned int i = 0; i < device_count; ++i) device_ids[i] = devices[i].raw_device_id();
      
      // Create the context
      cl_int error_code;
      if(internal_callback_ptr) {
         internal_id = clCreateContext(properties.opencl_view(), device_count, device_ids, raw_callback, static_cast<void *>(internal_callback_ptr), &error_code);
      } else {
         internal_id = clCreateContext(properties.opencl_view(), device_count, device_ids, nullptr, nullptr, &error_code);
      }

      // Signal any erro that occured during context creation
      throw_if_failed(error_code);
   }

   void Context::create_context_from_type(ContextProperties & properties, const cl_device_type device_type) {
      // Create the context from a given device type
      cl_int error_code;
      if(internal_callback_ptr) {
         internal_id = clCreateContextFromType(properties.opencl_view(), device_type, raw_callback, static_cast<void *>(internal_callback_ptr), &error_code);
      } else {
         internal_id = clCreateContextFromType(properties.opencl_view(), device_type, nullptr, nullptr, &error_code);
      }

      // Signal any erro that occured during context creation
      if(error_code != 0) throw_standard_exception(error_code);
   }

   void Context::copy_internal_data(const Context & source) {
      internal_id = source.internal_id;
      internal_callback_ptr = source.internal_callback_ptr;
   }

   void Context::retain_context() {
      throw_if_failed(clRetainContext(internal_id));
   }

   void Context::release_context() {
      bool last_reference = (reference_count() == 1);
      throw_if_failed(clReleaseContext(internal_id));
      if(last_reference && internal_callback_ptr) delete internal_callback_ptr;
   }

}
