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

#ifndef INCLUDE_CL_PLUSPLUS_EVENT
#define INCLUDE_CL_PLUSPLUS_EVENT

#include <array>
#include <functional>
#include <memory>
#include <vector>

#include <CL/cl.h>

#include "common.hpp"

// This code unit provides a high-level way to manage OpenCL events
namespace CLplusplus {

   // This class can directly manage any OpenCL event, including user events.
   class Event {
      public:
         // === BASIC CLASS LIFECYCLE ===

         // Events can be created from a valid OpenCL identifier
         Event(const cl_event identifier, const bool increment_reference_count);

         // Events are reference counted using the following functions
         Event(const Event & source);
         ~Event() { release(); }
         Event & operator=(const Event & source);

         // === PROPERTIES ===

         // Event properties which are supported by the wrapper are directly accessible in a convenient, high-level fashion
         cl_command_type command_type() const { return raw_value_query<cl_command_type>(CL_EVENT_COMMAND_TYPE); }
         cl_int check_command_execution_status() const; // NOTE : This version will throw an exception if a command has failed to execute, so it can only return positive status codes.

         // Unsupported property values may be queried in a lower-level way
         cl_command_queue raw_command_queue_id() const { return raw_value_query<cl_command_queue>(CL_EVENT_COMMAND_QUEUE); } // NOTE : Returning a CommandQueue here would lead to a circular dependency.
         cl_context raw_context_id() const { return raw_value_query<cl_context>(CL_EVENT_CONTEXT); } // WARNING : Beware that trying to use this identifier in a Context can lead to callback memory leaks.
         cl_int raw_command_execution_status() const { return raw_value_query<cl_int>(CL_EVENT_COMMAND_EXECUTION_STATUS); }

         // And fully unsupported event properties can be queried in a nearly pure OpenCL way, with some common-case usability optimizations
         template<typename ValueType> ValueType raw_value_query(const cl_event_info parameter_name) const {
            ValueType result;
            raw_query(parameter_name, sizeof(ValueType), &result);
            return result;
         }

         size_t raw_query_output_size(const cl_event_info parameter_name) const;
         void raw_query(const cl_event_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size = nullptr) const;

         // === CALLBACKS ===

         // It is possible to have a callback be invoked when the command associated to an event will be submitted (CL_SUBMIT), running (CL_RUNNING) or terminated (CL_COMPLETE).
         // Users of this functionality should keep in mind that the callback associated with CL_COMPLETE will also be invoked if a command terminates abnormally, with an error code as its argument.
         // Another thing to keep in mind is that callbacks are stored within the Event object, so users should make sure that an Event with a callback has been either full processed or copied before leaving its scope.
         //
         // We accept native std::functions for this purpose, with and without user-defined data blocks.
         // We discourage the use of such data blocks in C++11 as lambdas and std::bind() usually provide a safer alternative, but they are needed for legacy C code compatibility.
         using EventCallback = std::function<void(cl_event, cl_int)>;
         using EventCallbackWithUserData = std::function<void(cl_event, cl_int, void *)>;

         // Here are the functions used to set such callbacks
         void set_callback(const cl_int command_exec_callback_type, const EventCallback & callback);
         void set_callback(const cl_int command_exec_callback_type, const EventCallbackWithUserData & callback, void * const user_data);
         class UnsupportedCallbackType : WrapperException {};

         // === USER EVENTS ===

         // User events can also have their status set to CL_COMPLETE or a negative error code exactly once in their lifetime
         void set_status(cl_int final_execution_status) const;

         // === PROFILING ===

         // Non-user events may be queried for supported profiling info on any command queue which has CL_QUEUE_PROFILING_ENABLE set, after they reach completion.
         cl_ulong enqueue_time_ns() const { return raw_profiling_ulong_query(CL_PROFILING_COMMAND_QUEUED); }
         cl_ulong submit_time_ns() const { return raw_profiling_ulong_query(CL_PROFILING_COMMAND_SUBMIT); }
         cl_ulong start_time_ns() const { return raw_profiling_ulong_query(CL_PROFILING_COMMAND_START); }
         cl_ulong end_time_ns() const { return raw_profiling_ulong_query(CL_PROFILING_COMMAND_END); }

         // And fully unsupported profiling info can be queried in a nearly pure OpenCL way, with some common-case usability optimizations
         cl_ulong raw_profiling_ulong_query(const cl_profiling_info parameter_name) const;
         size_t raw_profiling_query_output_size(const cl_profiling_info parameter_name) const;
         void raw_profiling_query(const cl_profiling_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size = nullptr) const;

         // === RAW OPENCL ID ===

         // Finally, if the need arises, one can directly access the event identifier in order to perform raw OpenCL operations.
         // WARNING : Be very careful when you do this, as such raw identifiers will NOT be taken into account during reference counting !
         cl_event raw_identifier() const { return internal_id; }

      private:
         // This is the internal identifier that represents our event object
         cl_event internal_id;

         // In general, we DO NOT want to deal with multiple kinds of callbacks, so the user data version is converted to a regular callback as soon as possible
         static EventCallback make_event_callback(const EventCallbackWithUserData & callback, void * const user_data);

         // High-level destructor callbacks are stored here. They will be called by lower-level static functions which follow OpenCL's linkage conventions.
         // We need multiple event lists and static callbacks because we can be waiting for multiple event states.
         using EventCallbackLists = std::array<std::vector<EventCallback>, 3>;
         std::shared_ptr<EventCallbackLists> internal_callbacks_ptr;
         void add_event_callback(const cl_int command_exec_callback_type, const EventCallback & callback);
         static void CL_CALLBACK raw_callback(cl_event event, cl_int event_command_exec_status, void * actual_callbacks_ptr);

         // These functions manage the life cycle of reference-counted memory objects
         void copy_internal_data(const Event & source);
         cl_uint reference_count() const { return raw_value_query<cl_uint>(CL_EVENT_REFERENCE_COUNT); }
         void retain() const;
         void release();
   };

   // This function may be used to wait on a number of pending OpenCL events.
   void wait_for_events(const std::vector<Event> & events);

}

#endif
