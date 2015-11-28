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

#ifndef INCLUDE_CL_PLUSPLUS_PROGRAM
#define INCLUDE_CL_PLUSPLUS_PROGRAM

#include <string>
#include <utility>
#include <vector>

#include <CL/cl.h>

#include "common.hpp"
#include "device.hpp"
#include "event.hpp"

// This code unit provides a high-level way to manage OpenCL program objects
namespace CLplusplus {

   // To avoid going through a slow compilation process, programs can be provided in a binary form. This is said form.
   using ProgramBinary = std::vector<unsigned char>;

   // This class represents an OpenCL program object, that can be queried in a high-level way.
   class Program {
      public:
         // === BASIC CLASS LIFECYCLE ===

         // Program objects can be created from a valid OpenCL identifier
         Program(const cl_program identifier, const bool increment_reference_count);

         // Program objects are reference counted using the following functions
         Program(const Program & source);
         ~Program() { release(); }
         Program & operator=(const Program & source);

         // === PROPERTIES ===

         // --- Global program object properties ---

         // Program properties which are supported by the wrapper are directly accessible in a convenient, high-level fashion
         cl_uint num_devices() const { return raw_uint_query(CL_PROGRAM_NUM_DEVICES); }
         std::vector<CLplusplus::Device> devices() const;

         std::string source() const { return raw_string_query(CL_PROGRAM_SOURCE); }

         std::vector<size_t> binary_sizes() const;
         std::vector<ProgramBinary> binaries() const;

         size_t num_kernels() const { return raw_value_query<size_t>(CL_PROGRAM_NUM_KERNELS); }
         std::vector<std::string> kernel_names() const { return decode_opencl_list(raw_string_query(CL_PROGRAM_KERNEL_NAMES), ';'); }

         // Unsupported property values may be queried in a lower-level way
         cl_context raw_context_id() const { return raw_value_query<cl_context>(CL_PROGRAM_CONTEXT); } // NOTE : Returning a Context here would lead to a circular dependency.
                                                                                                       // WARNING : Beware that trying to use this identifier in a Context can lead to callback memory leaks.
         void raw_get_binaries(const size_t device_amount, unsigned char * dest_storage[]) const;

         // And fully unsupported program properties can be queried in a nearly pure OpenCL way, with some common-case usability optimizations
         std::string raw_string_query(const cl_program_info parameter_name) const;
         cl_uint raw_uint_query(const cl_program_info parameter_name) const { return raw_value_query<cl_uint>(parameter_name); }

         template<typename ValueType> ValueType raw_value_query(const cl_program_info parameter_name) const {
            ValueType result;
            raw_query(parameter_name, sizeof(ValueType), &result);
            return result;
         }

         size_t raw_query_output_size(const cl_program_info parameter_name) const;
         void raw_query(const cl_program_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size = nullptr) const;

         // --- Per-device program build info ---

         // Program build info which is supported by the wrapper is directly accessible in a convenient, high-level fashion
         cl_build_status build_status(const Device & device) const { return raw_build_info_value_query<cl_build_status>(device, CL_PROGRAM_BUILD_STATUS); }
         std::string build_options(const Device & device) const { return raw_build_info_string_query(device, CL_PROGRAM_BUILD_OPTIONS); }
         std::string build_log(const Device & device) const { return raw_build_info_string_query(device, CL_PROGRAM_BUILD_LOG); }
         cl_program_binary_type binary_type(const Device & device) const { return raw_build_info_value_query<cl_program_binary_type>(device, CL_PROGRAM_BINARY_TYPE); }

         // And fully unsupported program build info can be queried in a nearly pure OpenCL way, with some common-case usability optimizations
         std::string raw_build_info_string_query(const Device & device, const cl_program_build_info parameter_name) const;

         template<typename ValueType> ValueType raw_build_info_value_query(const Device & device, const cl_program_build_info parameter_name) const {
            ValueType result;
            raw_build_info_query(device, parameter_name, sizeof(ValueType), &result);
            return result;
         }

         size_t raw_build_info_query_output_size(const Device & device, const cl_program_build_info parameter_name) const;
         void raw_build_info_query(const Device & device, const cl_program_build_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size = nullptr) const;

         // === BUILDING EXECUTABLES ===

         // For all program building commands, we accept native std::functions as callbacks, with and without user-defined data blocks.
         // We discourage the use of such data blocks in C++11 as lambdas and std::bind() usually provide a safer alternative, but they are needed for legacy C code compatibility.
         // Another thing to keep in mind is that callbacks are stored within the Program object, so users should make sure that a Program with a callback has been either built or copied before leaving its scope.
         //
         // Finally, please consider using our event-based asynchronous build functionality instead of callbacks. It's built on top of them, but more idiomatic OpenCL and less error-prone to use.
         using BuildCallback = std::function<void(cl_program)>;
         using BuildCallbackWithUserData = std::function<void(cl_program, void *)>;

         // One can build programs either for all associated devices...
         CLplusplus::Event build_with_event(const std::string & options);
         void build(const std::string & options, const BuildCallback & callback = nullptr);
         void build(const std::string & options, const BuildCallbackWithUserData & callback, void * const user_data);

         // ...or for a selection of devices only
         CLplusplus::Event build_with_event(const std::vector<Device> & device_list, const std::string & options);
         void build(const std::vector<Device> & device_list, const std::string & options, const BuildCallback & callback = nullptr);
         void build(const std::vector<Device> & device_list, const std::string & options, const BuildCallbackWithUserData & callback, void * const user_data);

         // === RAW OPENCL ID ===

         // Finally, if the need arises, one can directly access the program object identifier in order to perform raw OpenCL operations.
         // WARNING : Be very careful when you do this, as such raw identifiers will NOT be taken into account during reference counting !
         cl_program raw_identifier() const { return internal_id; }

      private:
         // This is the internal identifier that represents our program object
         cl_program internal_id;

         // In general, we DO NOT want to deal with multiple kinds of callbacks, so the user data version is converted to a regular callback as soon as possible
         static BuildCallback make_build_callback(const BuildCallbackWithUserData & callback, void * const user_data);

         // Similarly, event-based asynchronous builds actually use a callback, which is defined here
         class UnsupportedBuildStatus : public WrapperException {};
         std::pair<Event, BuildCallback> make_build_event_callback(const std::vector<Device> * const device_list_ptr) const;

         // High-level context callbacks are stored here. They will be called by a lower-level static function which follows OpenCL's linkage conventions.
         BuildCallback * internal_callback_ptr;
         static void CL_CALLBACK raw_callback(cl_program program, void * program_object);

         // This lower-level function eliminates code duplication in program build code
         void raw_build_program(const std::vector<Device> * const device_list_ptr, const std::string & options, const BuildCallback & callback);

         // These functions manage the life cycle of reference-counted program objects
         void copy_internal_data(const Program & source);
         cl_uint reference_count() const { return raw_uint_query(CL_PROGRAM_REFERENCE_COUNT); }
         void retain() const;
         void release();
   };

}

#endif
