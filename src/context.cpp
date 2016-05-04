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

#include <algorithm>
#include <functional>
#include <fstream>

#include "common.hpp"
#include "context.hpp"

namespace CLplusplus {

   Context::Context(const cl_context identifier, const bool increment_reference_count) :
      internal_id{identifier},
      internal_callback_ptr{nullptr}
   {
      // Handle invalid context IDs
      if(internal_id == NULL) throw InvalidArgument();
      
      // Unless asked not to do so, increment the context's reference count
      if(increment_reference_count) retain();
   }

   Context::Context(ContextProperties & properties, const Device & device, const ContextCallback & callback) :
      single_device_id{device.raw_identifier()},
      internal_callback_ptr{callback ? std::make_shared<ContextCallback>(callback) : nullptr}
   {
      create_context(properties, 1, &device);
   }

   Context::Context(ContextProperties & properties, const Device & device, const ContextCallbackWithUserData & callback, void * const user_data) :
      single_device_id{device.raw_identifier()},
      internal_callback_ptr{callback ? std::make_shared<ContextCallback>(make_context_callback(callback, user_data)) : nullptr}
   {
      create_context(properties, 1, &device);
   }

   Context::Context(ContextProperties & properties, const std::vector<Device> & devices, const ContextCallback & callback) :
      single_device_id{NULL},
      internal_callback_ptr{callback ? std::make_shared<ContextCallback>(callback) : nullptr}
   {
      create_context(properties, devices.size(), &devices[0]);
   }

   Context::Context(ContextProperties & properties, const std::vector<Device> & devices, const ContextCallbackWithUserData & callback, void * const user_data) :
      single_device_id{NULL},
      internal_callback_ptr{callback ? std::make_shared<ContextCallback>(make_context_callback(callback, user_data)) : nullptr}
   {
      create_context(properties, devices.size(), &devices[0]);
   }

   Context::Context(ContextProperties & properties, const cl_device_type device_type, const ContextCallback & callback) :
      single_device_id{NULL},
      internal_callback_ptr{callback ? std::make_shared<ContextCallback>(callback) : nullptr}
   {
      create_context_from_type(properties, device_type);
   }

   Context::Context(ContextProperties & properties, const cl_device_type device_type, const ContextCallbackWithUserData & callback, void * const user_data) :
      single_device_id{NULL},
      internal_callback_ptr{callback ? std::make_shared<ContextCallback>(make_context_callback(callback, user_data)) : nullptr}
   {
      create_context_from_type(properties, device_type);
   }

   Context::Context(const Context & source) {
      // Whenever a copy of a reference-counted context is made, its reference count should be incremented
      copy_internal_data(source);
      retain();
   }

   Context & Context::operator=(const Context & source) {
      // Reference count considerations also apply to copy assignment operator
      if(source.internal_id == internal_id) return *this;
      release();
      copy_internal_data(source);
      retain();
      return *this;
   }

   std::vector<Device> Context::devices() const {
      // Check how many devices are connected to the context and allocate storage accordingly
      const auto device_amount = num_devices();
      cl_device_id raw_device_ids[device_amount];

      // Request the context to provide a device ID list
      raw_query(CL_CONTEXT_DEVICES, device_amount * sizeof(cl_device_id), static_cast<void *>(raw_device_ids));

      // Convert it into high-level output
      std::vector<Device> result;
      result.reserve(device_amount);
      for(unsigned int i = 0; i < device_amount; ++i) result.emplace_back(Device{raw_device_ids[i], true});
      return result;
   }

   ContextProperties Context::properties() const {
      // Check the length of the null-terminated OpenCL property list and allocate storage accordingly
      const auto property_list_length = raw_query_output_size(CL_CONTEXT_PROPERTIES) / sizeof(cl_context_properties);
      cl_context_properties raw_properties[property_list_length];

      // Request the context to provide the property list
      raw_query(CL_CONTEXT_PROPERTIES, property_list_length * sizeof(cl_context_properties), (void *)raw_properties);

      // Convert it into high-level output
      return ContextProperties{raw_properties};
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

   CLplusplus::CommandQueue Context::create_command_queue(const Device & device, const cl_command_queue_properties properties) const {
      return raw_create_command_queue(device.raw_identifier(), properties);
   }

   CLplusplus::CommandQueue Context::create_command_queue(const cl_command_queue_properties properties) const {
      // If our context was created, or could have been created, with more than one device, then this call is invalid
      if(single_device_id == NULL) throw AmbiguousDevice();

      // Otherwise, create the command queue using the previously stored device id
      return raw_create_command_queue(single_device_id, properties);
   }

   CLplusplus::Buffer Context::create_buffer(const cl_mem_flags flags, const size_t size, void * const host_ptr) const {
      cl_int error_code;
      const auto buffer_id = clCreateBuffer(internal_id, flags, size, host_ptr, &error_code);
      throw_if_failed(error_code);
      return Buffer{buffer_id, false};
   }

   #ifdef CL_VERSION_1_2
   CLplusplus::Image Context::create_image(const cl_mem_flags flags, const cl_image_format & image_format, const cl_image_desc & image_desc, void * const host_ptr) const {
      cl_int error_code;
      const auto image_id = clCreateImage(internal_id, flags, &image_format, &image_desc, host_ptr, &error_code);
      throw_if_failed(error_code);
      return Image{image_id, false};
   }
   #endif

   std::vector<cl_image_format> Context::supported_image_formats(const cl_mem_flags flags, const cl_mem_object_type image_type) const {
      // Query how many image formats there will be, and prepare a vector of suitable size
      cl_uint num_image_formats;
      throw_if_failed(clGetSupportedImageFormats(internal_id, flags, image_type, 0, nullptr, &num_image_formats));
      std::vector<cl_image_format> result(num_image_formats);

      // Fetch the list of supported image formats and return it
      throw_if_failed(clGetSupportedImageFormats(internal_id, flags, image_type, num_image_formats, &(result[0]), nullptr));
      return result;
   }

   CLplusplus::Program Context::create_program_with_source(const std::string & source_code) const {
      cl_int error_code;
      const size_t source_code_length = source_code.size();
      const char * source_code_str = &(source_code[0]);
      const auto program_id = clCreateProgramWithSource(internal_id, 1, &source_code_str, &source_code_length, &error_code);
      throw_if_failed(error_code);
      return Program{program_id, false};
   }

   CLplusplus::Program Context::create_program_with_source_file(const std::string & source_code_filename) const {
      // Open source file and determine its size
      std::ifstream input_file(source_code_filename, std::ios_base::in | std::ios_base::ate);
      if(input_file.fail()) throw FileOpenFailed();
      const auto file_size = input_file.tellg();
      input_file.seekg(0);

      // Create a string large enough to store the file's contents
      std::string source_code;
      source_code.resize(file_size);

      // Load the source file's contents into the string
      input_file.read(&(source_code[0]), file_size);

      // Proceed in the normal way for source strings
      return create_program_with_source(source_code);
   }

   CLplusplus::Program Context::create_program_with_binary(const std::vector<Device> & device_list, const std::vector<ProgramBinary> & binaries, cl_int * const binaries_status) const {
      const size_t num_devices = device_list.size();
      cl_device_id raw_device_ids[num_devices];
      for(size_t i = 0; i < num_devices; ++i) raw_device_ids[i] = device_list[i].raw_identifier();
      return raw_create_program_with_binary(num_devices, raw_device_ids, binaries, binaries_status);
   }

   CLplusplus::Program Context::create_program_with_binary(const ProgramBinary & binary) const {
      // If our context was created, or could have been created, with more than one device, then this call is invalid
      if(single_device_id == NULL) throw AmbiguousDevice();

      // Otherwise, create the program using the previously stored device id
      return raw_create_program_with_binary(1, &single_device_id, {binary}, nullptr);
   }

   #ifdef CL_VERSION_1_2
   CLplusplus::Program Context::create_program_with_built_in_kernels(const std::vector<Device> & device_list, const std::vector<std::string> & kernel_names) const {
      const size_t num_devices = device_list.size();
      cl_device_id raw_device_ids[num_devices];
      for(size_t i = 0; i < num_devices; ++i) raw_device_ids[i] = device_list[i].raw_identifier();
      return raw_create_program_with_built_in_kernels(num_devices, raw_device_ids, kernel_names);
   }

   CLplusplus::Program Context::create_program_with_built_in_kernels(const std::vector<std::string> & kernel_names) const {
      // If our context was created, or could have been created, with more than one device, then this call is invalid
      if(single_device_id == NULL) throw AmbiguousDevice();

      // Otherwise, create the program using the previously stored device id
      return raw_create_program_with_built_in_kernels(1, &single_device_id, kernel_names);
   }
   #endif

   Event Context::create_user_event() const {
      cl_int error_code;
      const auto event_id = clCreateUserEvent(internal_id, &error_code);
      throw_if_failed(error_code);
      return Event{event_id, false};
   }

   Context::ContextCallback Context::make_context_callback(const ContextCallbackWithUserData & callback, void * const user_data) {
      using namespace std::placeholders;
      return ContextCallback{std::bind(callback, _1, _2, _3, user_data)};
   }

   void CL_CALLBACK Context::raw_callback(const char * errinfo, const void * private_info, size_t cb, void * actual_callback_ptr) {
      const auto actual_callback = *(static_cast<const ContextCallback *>(actual_callback_ptr));
      actual_callback(std::string{errinfo}, private_info, cb);
   }

   void Context::create_context(ContextProperties & properties, const cl_uint device_count, const Device * devices) {
      // Extract the internal identifiers of our devices
      cl_device_id device_ids[device_count];
      for(unsigned int i = 0; i < device_count; ++i) device_ids[i] = devices[i].raw_identifier();
      
      // Create the context
      cl_int error_code;
      if(internal_callback_ptr) {
         internal_id = clCreateContext(properties.opencl_view(), device_count, device_ids, raw_callback, static_cast<void *>(&*internal_callback_ptr), &error_code);
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
         internal_id = clCreateContextFromType(properties.opencl_view(), device_type, raw_callback, static_cast<void *>(&*internal_callback_ptr), &error_code);
      } else {
         internal_id = clCreateContextFromType(properties.opencl_view(), device_type, nullptr, nullptr, &error_code);
      }

      // Signal any erro that occured during context creation
      if(error_code != 0) throw_standard_exception(error_code);
   }

   CommandQueue Context::raw_create_command_queue(const cl_device_id device_id, const cl_command_queue_properties properties) const {
      cl_int error_code;
      const auto command_queue_id = clCreateCommandQueue(internal_id, device_id, properties, &error_code);
      throw_if_failed(error_code);
      return CommandQueue{command_queue_id, false};
   }

   Program Context::raw_create_program_with_binary(const size_t num_devices, const cl_device_id * const raw_device_ids, const std::vector<ProgramBinary> & binaries, cl_int * const binaries_status) const {
      // Create a C-compatible representation of the binary list
      size_t binary_lengths[num_devices];
      for(size_t i = 0; i < num_devices; ++i) binary_lengths[i] = binaries[i].size();
      const unsigned char * raw_binaries[num_devices];
      for(size_t i = 0; i < num_devices; ++i) raw_binaries[i] = &(binaries[i][0]);

      // Create the program
      cl_int error_code;
      const auto program_id = clCreateProgramWithBinary(internal_id, num_devices, raw_device_ids, binary_lengths, raw_binaries, binaries_status, &error_code);
      throw_if_failed(error_code);
      return Program{program_id, false};
   }

   #ifdef CL_VERSION_1_2
   Program Context::raw_create_program_with_built_in_kernels(const size_t num_devices, const cl_device_id * const raw_device_ids, const std::vector<std::string> & kernel_names) const {
      // Determine how long a semicolon-separated kernel list should be
      size_t kernel_list_size = kernel_names.size() - 1;
      for(const auto & kernel_name : kernel_names) kernel_list_size += kernel_name.length();

      // Build such a kernel list
      std::string kernel_list;
      kernel_list.reserve(kernel_list_size);
      for(size_t i = 0; i < num_devices - 1; ++i) {
         kernel_list.append(kernel_names[i]);
         kernel_list.push_back(';');
      }
      kernel_list.append(kernel_names[num_devices - 1]);

      // Create the program
      cl_int error_code;
      const auto program_id = clCreateProgramWithBuiltInKernels(internal_id, num_devices, raw_device_ids, &(kernel_list[0]), &error_code);
      throw_if_failed(error_code);
      return Program{program_id, false};
   }
   #endif

   void Context::copy_internal_data(const Context & source) {
      internal_id = source.internal_id;
      single_device_id = source.single_device_id;
      internal_callback_ptr = source.internal_callback_ptr;
   }

   void Context::retain() const {
      throw_if_failed(clRetainContext(internal_id));
   }

   void Context::release() {
      throw_if_failed(clReleaseContext(internal_id));
   }

}
