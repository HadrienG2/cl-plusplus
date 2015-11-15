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
#include "memory_object.hpp"

namespace CLplusplus {

   MemoryObject::MemoryObject(const MemoryObject & source) {
      // Whenever a copy of a reference-counted memory object is made, its reference count should be incremented
      copy_internal_data(source);
      retain();
   }

   MemoryObject & MemoryObject::operator=(const MemoryObject & source) {
      // Reference count considerations also apply to copy assignment operator
      copy_internal_data(source);
      retain();
      return *this;
   }

   void MemoryObject::set_destructor_callback(const DestructorCallback & callback) {
      add_destructor_callback(callback);
   }

   void MemoryObject::set_destructor_callback(const DestructorCallbackWithUserData & callback, void * const user_data) {
      add_destructor_callback(make_destructor_callback(callback, user_data));
   }

   size_t MemoryObject::raw_query_output_size(const cl_mem_info parameter_name) const {
      size_t result;
      raw_query(parameter_name, 0, nullptr, &result);
      return result;
   }

   void MemoryObject::raw_query(const cl_mem_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size) const {
      throw_if_failed(clGetMemObjectInfo(internal_id, parameter_name, output_storage_size, output_storage, actual_output_size));
   }

   MemoryObject::MemoryObject(const cl_mem identifier, const bool increment_reference_count) :
      internal_id{identifier},
      internal_callbacks_ptr{new std::vector<DestructorCallback>}
   {
      // Handle invalid memory object IDs
      if(internal_id == NULL) throw InvalidArgument();
      
      // Unless asked not to do so, increment the memory object's reference count
      if(increment_reference_count) retain();
   }

   MemoryObject::DestructorCallback MemoryObject::make_destructor_callback(const DestructorCallbackWithUserData & callback, void * const user_data) {
      using namespace std::placeholders;
      return DestructorCallback{std::bind(callback, _1, user_data)};
   }

   void MemoryObject::add_destructor_callback(const DestructorCallback & callback) {
      // First, we must add the destructor callback to our shared callback database
      if(callback) internal_callbacks_ptr->emplace_back(callback);

      // If we just added our first destructor callback, we must also register our catch-all static callback to OpenCL
      if(internal_callbacks_ptr->size() == 1) {
         throw_if_failed(clSetMemObjectDestructorCallback(internal_id, raw_callback, static_cast<void *>(internal_callbacks_ptr)));
      }
   }

   void CL_CALLBACK MemoryObject::raw_callback(cl_mem memobj, void * actual_callbacks_ptr) {
      // Extract the destructor callback vector from the raw void * pointer we received
      const auto actual_callbacks = *(static_cast<const std::vector<DestructorCallback> *>(actual_callbacks_ptr));

      // Call the destructor callbacks in reverse order (per OpenCL specification)
      for(int i = actual_callbacks.size() - 1; i >= 0; --i) {
         actual_callbacks[i](memobj);
      }
   }

   void MemoryObject::copy_internal_data(const MemoryObject & source) {
      internal_id = source.internal_id;
      internal_callbacks_ptr = source.internal_callbacks_ptr;
   }

   void MemoryObject::retain() const {
      throw_if_failed(clRetainMemObject(internal_id));
   }

   void MemoryObject::release() {
      bool last_reference = (reference_count() == 1);
      throw_if_failed(clReleaseMemObject(internal_id));
      if(last_reference && internal_callbacks_ptr) delete internal_callbacks_ptr;
   }

}
