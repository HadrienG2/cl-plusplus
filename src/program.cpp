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
#include "context.hpp"
#include "program.hpp"

namespace CLplusplus {

   Program::Program(const cl_program identifier, const bool increment_reference_count) :
      internal_id{identifier},
      internal_callback_ptr{std::make_shared<std::unique_ptr<BuildCallback>>()}
   {
      // Handle invalid program IDs
      if(internal_id == NULL) throw InvalidArgument();
      
      // Unless asked not to do so, increment the program object's reference count
      if(increment_reference_count) retain();
   }

   Program::Program(const Program & source) {
      // Whenever a copy of a reference-counted program object is made, its reference count should be incremented
      copy_internal_data(source);
      retain();
   }

   Program & Program::operator=(const Program & source) {
      // Reference count considerations also apply to copy assignment operator
      if(source.internal_id == internal_id) return *this;
      release();
      copy_internal_data(source);
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
      result.reserve(device_amount);
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

   std::string Program::raw_build_info_string_query(const Device & device, const cl_program_build_info parameter_name) const {
      // Check how long the output string should be
      size_t output_string_length = raw_build_info_query_output_size(device, parameter_name);
      
      // Fetch the output string
      char output_string[output_string_length];
      raw_build_info_query(device, parameter_name, output_string_length, static_cast<void *>(output_string));

      // Return the result
      return std::string(output_string);
   }

   size_t Program::raw_build_info_query_output_size(const Device & device, const cl_program_build_info parameter_name) const {
      size_t result;
      raw_build_info_query(device, parameter_name, 0, nullptr, &result);
      return result;
   }

   void Program::raw_build_info_query(const Device & device, const cl_program_build_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size) const {
      throw_if_failed(clGetProgramBuildInfo(internal_id, device.raw_identifier(), parameter_name, output_storage_size, output_storage, actual_output_size));
   }

   CLplusplus::Event Program::build_with_event(const std::string & options) {
      const auto event_and_callback = make_build_event_callback(nullptr);
      raw_build_program(nullptr, options, event_and_callback.second);
      return event_and_callback.first;
   }

   void Program::build(const std::string & options, const BuildCallback & callback) {
      raw_build_program(nullptr, options, callback);
   }

   void Program::build(const std::string & options, const BuildCallbackWithUserData & callback, void * const user_data) {
      raw_build_program(nullptr, options, make_build_callback(callback, user_data));
   }

   CLplusplus::Event Program::build_with_event(const std::vector<Device> & device_list, const std::string & options) {
      const auto event_and_callback = make_build_event_callback(&device_list);
      raw_build_program(&device_list, options, event_and_callback.second);
      return event_and_callback.first;
   }

   void Program::build(const std::vector<Device> & device_list, const std::string & options, const BuildCallback & callback) {
      raw_build_program(&device_list, options, callback);
   }

   void Program::build(const std::vector<Device> & device_list, const std::string & options, const BuildCallbackWithUserData & callback, void * const user_data) {
      raw_build_program(&device_list, options, make_build_callback(callback, user_data));
   }

   CLplusplus::Kernel Program::create_kernel(const std::string & kernel_name) const {
      cl_int error_code;
      const auto kernel_id = clCreateKernel(internal_id, kernel_name.c_str(), &error_code);
      throw_if_failed(error_code);
      return Kernel{kernel_id, false};
   }

   CLplusplus::Kernel Program::create_kernel(const std::string & kernel_name, const Event & program_build_event) const {
      wait_for_events({program_build_event});
      return create_kernel(kernel_name);
   }

   std::vector<CLplusplus::Kernel> Program::create_kernels_in_program() const {
      // Determine how many kernels we are going to generate
      cl_uint num_kernels;
      throw_if_failed(clCreateKernelsInProgram(internal_id, 0, nullptr, &num_kernels));

      // Fetch the associated raw kernel identifiers
      cl_kernel kernel_ids[num_kernels];
      throw_if_failed(clCreateKernelsInProgram(internal_id, num_kernels, kernel_ids, nullptr));

      // Build a table of high-level kernel objects from these identifiers
      std::vector<Kernel> result;
      result.reserve(num_kernels);
      for(size_t i = 0; i < num_kernels; ++i) result.emplace_back(Kernel{kernel_ids[i], false});
      return result;
   }

   std::vector<CLplusplus::Kernel> Program::create_kernels_in_program(const Event & program_build_event) const {
      wait_for_events({program_build_event});
      return create_kernels_in_program();
   }

   Program::BuildCallback Program::make_build_callback(const BuildCallbackWithUserData & callback, void * const user_data) {
      using namespace std::placeholders;
      return BuildCallback{std::bind(callback, _1, user_data)};
   }

   std::pair<Event, Program::BuildCallback> Program::make_build_event_callback(const std::vector<Device> * const device_list_ptr) const {
      // Save the list of devices that our asynchronous build process will be concerned about
      const auto device_list = (device_list_ptr) ? (*device_list_ptr) : devices();

      // Create a user event within the context that our program belongs to
      const auto context = Context{raw_context_id(), true};
      const auto user_event = context.create_user_event();

      // Prepare a callback that will set the user event according to the program build status
      const auto active_program_reference = *this;
      const auto callback = [user_event, device_list, active_program_reference](cl_program unused) {
         // For each device, check if the build went well. If not, notify the event users and abort.
         for(const auto & device : device_list) {
            const auto build_status = active_program_reference.build_status(device);
            if(build_status == CL_BUILD_SUCCESS) continue;
            switch(build_status) {
               case CL_BUILD_ERROR:
                  user_event.set_status(CL_BUILD_PROGRAM_FAILURE);
                  return;
               default:
                  throw UnsupportedBuildStatus();
            }
         }

         // If everything went well for all devices, set a positive build event status.
         user_event.set_status(CL_COMPLETE);
      };

      // Return our user event and the callback that will ultimately set it
      return {user_event, callback};
   }

   void CL_CALLBACK Program::raw_callback(cl_program program, void * program_object_ptr) {
      // Fetch our program object
      auto & program_object = *(static_cast<Program *>(program_object_ptr));
      
      // Call its previously saved build callback
      auto &callback_ptr = *program_object.internal_callback_ptr;
      const auto actual_callback = *callback_ptr;
      actual_callback(program);

      // Destroy the build callback
      callback_ptr.reset();
   }

   void Program::raw_build_program(const std::vector<Device> * const device_list_ptr, const std::string & options, const BuildCallback & callback) {
      // To avoid callback memory leaks, raise InvalidOperation ourselves if a build is already occuring (rather than having OpenCL do it for us)
      auto & callback_ptr = *internal_callback_ptr;
      if(callback_ptr) throw StandardExceptions::InvalidOperation();

      // Save the program build callback and build event, if any
      if(callback) callback_ptr.reset( new BuildCallback{callback} );

      // Build the program, creating an OpenCL-compatible view of the device list if necessary
      try {
         if(device_list_ptr == nullptr) {
            if(callback) {
               throw_if_failed(clBuildProgram(internal_id, 0, nullptr, options.c_str(), raw_callback, (void *)this));
            } else {
               throw_if_failed(clBuildProgram(internal_id, 0, nullptr, options.c_str(), nullptr, nullptr));
            }
         } else {
            const auto & device_list = *device_list_ptr;
            const auto num_devices = device_list.size();
            cl_device_id raw_device_ids[num_devices];
            for(size_t i = 0; i < num_devices; ++i) raw_device_ids[i] = device_list[i].raw_identifier();
            if(callback) {
               throw_if_failed(clBuildProgram(internal_id, num_devices, raw_device_ids, options.c_str(), raw_callback, (void *)this));
            } else {
               throw_if_failed(clBuildProgram(internal_id, num_devices, raw_device_ids, options.c_str(), nullptr, nullptr));
            }
         }
      } catch(...) {
         callback_ptr.reset();
         throw;
      }
   }

   void Program::copy_internal_data(const Program & source) {
      internal_id = source.internal_id;
      internal_callback_ptr = source.internal_callback_ptr;
   }

   void Program::retain() const {
      throw_if_failed(clRetainProgram(internal_id));
   }

   void Program::release() {
      bool last_reference = (reference_count() == 1);
      throw_if_failed(clReleaseProgram(internal_id));
   }

}
