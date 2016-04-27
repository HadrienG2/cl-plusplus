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
#include "event.hpp"

namespace CLplusplus {

   Event::Event(const cl_event identifier, const bool increment_reference_count) :
      internal_id{identifier},
      internal_callbacks_ptr{new std::vector<EventCallback>[3]}
   {
      // Handle invalid event IDs
      if(internal_id == NULL) throw InvalidArgument();
      
      // Unless asked not to do so, increment the event object's reference count
      if(increment_reference_count) retain();
   }

   Event::Event(const Event & source) {
      // Whenever a copy of a reference-counted memory object is made, its reference count should be incremented
      copy_internal_data(source);
      retain();
   }

   Event & Event::operator=(const Event & source) {
      // Reference count considerations also apply to copy assignment operator
      if(source.internal_id == internal_id) return *this;
      release();
      copy_internal_data(source);
      retain();
      return *this;
   }

   size_t Event::raw_query_output_size(const cl_event_info parameter_name) const {
      size_t result;
      raw_query(parameter_name, 0, nullptr, &result);
      return result;
   }

   void Event::raw_query(const cl_event_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size) const {
      throw_if_failed(clGetEventInfo(internal_id, parameter_name, output_storage_size, output_storage, actual_output_size));
   }

   void Event::set_callback(const cl_int command_exec_callback_type, const EventCallback & callback) {
      add_event_callback(command_exec_callback_type, callback);
   }

   void Event::set_callback(const cl_int command_exec_callback_type, const EventCallbackWithUserData & callback, void * const user_data) {
      add_event_callback(command_exec_callback_type, make_event_callback(callback, user_data));
   }

   void Event::set_status(cl_int final_execution_status) const {
      throw_if_failed(clSetUserEventStatus(internal_id, final_execution_status));
   }

   cl_ulong Event::raw_profiling_ulong_query(const cl_profiling_info parameter_name) const {
      cl_ulong result;
      raw_profiling_query(parameter_name, sizeof(cl_ulong), static_cast<void *>(&result));
      return result;
   }

   size_t Event::raw_profiling_query_output_size(const cl_profiling_info parameter_name) const {
      size_t result;
      raw_profiling_query(parameter_name, 0, nullptr, &result);
      return result;
   }

   void Event::raw_profiling_query(const cl_profiling_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size) const {
      throw_if_failed(clGetEventProfilingInfo(internal_id, parameter_name, output_storage_size, output_storage, actual_output_size));
   }

   Event::EventCallback Event::make_event_callback(const EventCallbackWithUserData & callback, void * const user_data) {
      using namespace std::placeholders;
      return EventCallback{std::bind(callback, _1, _2, user_data)};
   }

   void Event::add_event_callback(const cl_int command_exec_callback_type, const EventCallback & callback) {
      // Pick which event callback store we are going to use
      unsigned int callback_store_index;
      switch(command_exec_callback_type) {
         case CL_SUBMITTED:
            callback_store_index = 0;
            break;
         case CL_RUNNING:
            callback_store_index = 1;
            break;
         case CL_COMPLETE:
            callback_store_index = 2;
            break;
         default:
            throw UnsupportedCallbackType();
      }
      auto & event_callback_store = internal_callbacks_ptr[callback_store_index];

      // Add our new event callback, if any, to the shared callback database
      if(callback) event_callback_store.emplace_back(callback);

      // If we just added our first destructor callback to this database, we must also register our catch-all static callback to OpenCL
      if(event_callback_store.size() == 1) {
         throw_if_failed(clSetEventCallback(internal_id, command_exec_callback_type, raw_callback, static_cast<void *>(&event_callback_store)));
      }
   }

   void CL_CALLBACK Event::raw_callback(cl_event event, cl_int event_command_exec_status, void * actual_callbacks_ptr) {
      // Extract the proper event callback vector from the raw void * pointer we received
      const auto actual_callbacks = *(static_cast<const std::vector<EventCallback> *>(actual_callbacks_ptr));

      // Call the destructor callbacks in reverse order (per OpenCL specification)
      for(const auto & callback : actual_callbacks) callback(event, event_command_exec_status);
   }

   void Event::copy_internal_data(const Event & source) {
      internal_id = source.internal_id;
      internal_callbacks_ptr = source.internal_callbacks_ptr;
   }

   void Event::retain() const {
      throw_if_failed(clRetainEvent(internal_id));
   }

   void Event::release() {
      bool last_reference = (reference_count() == 1);
      throw_if_failed(clReleaseEvent(internal_id));
      if(last_reference && internal_callbacks_ptr) delete[] internal_callbacks_ptr;
   }

   void wait_for_events(const std::vector<Event> & events) {
      // Build a list of raw events IDs to be waited on
      const auto num_events = events.size();      
      cl_event raw_event_ids[num_events];
      for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = events[i].raw_identifier();

      // Wait for these events to complete or for something to fail
      throw_if_failed(clWaitForEvents(num_events, raw_event_ids));
   }

}
