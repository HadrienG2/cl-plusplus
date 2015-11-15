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

#ifndef INCLUDE_CL_PLUSPLUS_CONTEXT
#define INCLUDE_CL_PLUSPLUS_CONTEXT

#include <functional>
#include <string>

#include <CL/cl.h>

#include "command_queue.hpp"
#include "device.hpp"
#include "event.hpp"
#include "property_list.hpp"

// This unit provides facilities for handling OpenCL contexts
namespace CLplusplus {

   // When an OpenCL context is created, some properties must be specified.
   // For this, OpenCL uses zero-terminated lists, which are relatively impractical to parse and a common source of security issues.
   // We propose an higher-level abstraction on top of these, see details in property_list.hpp
   using ContextProperties = CLplusplus::PropertyList<cl_context_properties>;

   // This class represents an OpenCL context, that can be queried in a high-level way.
   class Context {
      public:
         // First of all, we can wrap an existing context of known C handle
         Context(const cl_context identifier, const bool increment_reference_count);

         // For all other context creation constructors, we accept native std::functions as callbacks, with and without user-defined data blocks.
         // We discourage the use of such data blocks in C++11 as lambdas and std::bind() usually provide a safer alternative, but they are needed for legacy C code compatibility.
         using ContextCallback = std::function<void(const std::string &, const void *, size_t)>;
         using ContextCallbackWithUserData = std::function<void(const std::string &, const void *, size_t, void *)>;

         // Create a context from a single device (common case usability optimization)
         Context(ContextProperties & properties, const Device & device, const ContextCallback & callback = nullptr);
         Context(ContextProperties & properties, const Device & device, const ContextCallbackWithUserData & callback, void * const user_data);

         // Create a context from multiple devices -- wraps clCreateContext
         Context(ContextProperties & properties, const std::vector<Device> & devices, const ContextCallback & callback = nullptr);
         Context(ContextProperties & properties, const std::vector<Device> & devices, const ContextCallbackWithUserData & callback, void * const user_data);

         // Create a context from all devices of a specific type -- wraps clCreateContextFromType
         Context(ContextProperties & properties, const cl_device_type device_type, const ContextCallback & callback = nullptr);
         Context(ContextProperties & properties, const cl_device_type device_type, const ContextCallbackWithUserData & callback, void * const user_data);

         // Contexts are reference counted using the following functions
         Context(const Context & source);
         ~Context() { release(); }
         Context & operator=(const Context & source);

         // Context properties which are supported by the wrapper are directly accessible in a convenient, high-level fashion
         cl_uint num_devices() const { return raw_uint_query(CL_CONTEXT_NUM_DEVICES); }
         std::vector<CLplusplus::Device> devices() const;
         ContextProperties properties() const;

         // And unsupported context properties can be queried in a nearly pure OpenCL way, with some common-case usability optimizations
         cl_uint raw_uint_query(const cl_context_info parameter_name) const;
         size_t raw_query_output_size(const cl_context_info parameter_name) const;
         void raw_query(const cl_context_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size = nullptr) const;

         // It is possible to spawn a command queue on a context, for a device within this context.
         // And in the common case where the OpenCL context only wraps a single device, we can make that argument implicit.
         // If the context *could* contain multiple devices, even if that is not the case for a specific program instance, an exception will be thrown.
         CommandQueue create_command_queue(const Device & device, const cl_command_queue_properties properties) const;
         CommandQueue create_command_queue(const cl_command_queue_properties properties) const;
         class AmbiguousDevice : WrapperException {};

         // Within the boundaries of a context, one may also create user-triggered OpenCL events, so as to control the execution of asynchronous OpenCL code
         Event create_user_event() const;

         // Finally, if the need arises, one can directly access the context identifier in order to perform raw OpenCL operations.
         // WARNING : Be very careful when you do this, as such raw identifiers will NOT be taken into account during reference counting !
         cl_context raw_identifier() const { return internal_id; }

      private:
         // This is the internal identifier that represents our context
         cl_context internal_id;

         // If our context was created with only one device, this will contain its OpenCL identifier, otherwise it will contain NULL
         cl_device_id single_device_id;

         // In general, we DO NOT want to deal with multiple kinds of callbacks, so the user data version is converted to a regular callback as soon as possible
         static ContextCallback make_context_callback(const ContextCallbackWithUserData & callback, void * const user_data);

         // High-level context callbacks are stored here. They will be called by a lower-level static function which follows OpenCL's linkage conventions.
         ContextCallback * internal_callback_ptr;
         static void CL_CALLBACK raw_callback(const char * errinfo, const void * private_info, size_t cb, void * actual_callback_ptr);

         // These functions represent the common code path of context creation constructors, whichever callback option is chosen.
         void create_context(ContextProperties & properties, const cl_uint device_count, const Device * devices);
         void create_context_from_type(ContextProperties & properties, const cl_device_type device_type);

         // These functions manage the life cycle of reference-counted contexts
         void copy_internal_data(const Context & source);
         cl_uint reference_count() const { return raw_uint_query(CL_CONTEXT_REFERENCE_COUNT); }
         void retain() const;
         void release();

         // This function works behind the scene to create a command queue from a raw device ID
         CommandQueue raw_create_command_queue(const cl_device_id device_id, const cl_command_queue_properties properties) const;
   };

}

#endif
