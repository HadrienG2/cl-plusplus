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

#ifndef INCLUDE_CL_PLUSPLUS_MEMORY_OBJECT
#define INCLUDE_CL_PLUSPLUS_MEMORY_OBJECT

#include <functional>
#include <vector>

#include <CL/cl.h>

// This code unit defines a base class for all OpenCL memory objects : buffers, images...
namespace CLplusplus {

   // This class provides a high-level interface to all OpenCL memory objects
   class MemoryObject {
      public:
         // Memory objects are reference counted using the following functions
         MemoryObject(const MemoryObject & source);
         virtual ~MemoryObject() { release(); }
         MemoryObject & operator=(const MemoryObject & source);

         // It is possible to register a stack of callbacks to be called when memory objects will be destroyed. This is typically useful for shared host buffer liberation.
         // We accept native std::functions for this purpose, with and without user-defined data blocks.
         // We discourage the use of such data blocks in C++11 as lambdas and std::bind() usually provide a safer alternative, but they are needed for legacy C code compatibility.
         using DestructorCallback = std::function<void(cl_mem)>;
         using DestructorCallbackWithUserData = std::function<void(cl_mem, void *)>;

         // Here are the functions used to set such callbacks
         void set_destructor_callback(const DestructorCallback & callback);
         void set_destructor_callback(const DestructorCallbackWithUserData & callback, void * const user_data);

         // Memory object properties which are supported by the wrapper are directly accessible in a convenient, high-level fashion
         cl_mem_object_type type() const { return raw_value_query<cl_mem_object_type>(CL_MEM_TYPE); }
         cl_mem_flags flags() const { return raw_value_query<cl_mem_flags>(CL_MEM_FLAGS); }
         size_t size() const { return raw_size_query(CL_MEM_SIZE); }
         void * host_ptr() const { return raw_value_query<void*>(CL_MEM_HOST_PTR); }
         cl_uint map_count() const { return raw_uint_query(CL_MEM_MAP_COUNT); }

         bool has_associated_memobject() const { return (raw_associated_memobject() != NULL); }
         CLplusplus::MemoryObject associated_memobject() const { return MemoryObject{raw_associated_memobject(), true}; }
         size_t offset() const { return raw_size_query(CL_MEM_OFFSET); }

         // Unsupported property values may be queried in a lower-level way
         cl_context raw_context_id() const { return raw_value_query<cl_context>(CL_QUEUE_CONTEXT); } // NOTE : Returning a Context here would lead to a circular dependency.
                                                                                                     // WARNING : Beware that trying to use this identifier in a Context can lead to callback memory leaks.
         cl_mem raw_associated_memobject() const { return raw_value_query<cl_mem>(CL_MEM_ASSOCIATED_MEMOBJECT); }

         // And unsupported memory object properties can be queried in a nearly pure OpenCL way, with some common-case usability optimizations
         size_t raw_size_query(const cl_mem_info parameter_name) const { return raw_value_query<size_t>(parameter_name); }
         cl_uint raw_uint_query(const cl_mem_info parameter_name) const { return raw_value_query<cl_uint>(parameter_name); }

         template<typename ValueType> ValueType raw_value_query(const cl_mem_info parameter_name) const {
            ValueType result;
            raw_query(parameter_name, sizeof(ValueType), &result);
            return result;
         }

         size_t raw_query_output_size(const cl_mem_info parameter_name) const;
         void raw_query(const cl_mem_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size = nullptr) const;

         // Finally, if the need arises, one can directly access the memory object identifier in order to perform raw OpenCL operations.
         // WARNING : Be very careful when you do this, as such raw identifiers will NOT be taken into account during reference counting !
         cl_mem raw_identifier() const { return internal_id; }

      protected:
         // This is the internal identifier that represents our memory object
         cl_mem internal_id;

         // Memory objects can be created from a valid OpenCL identifier, but we should let child classes take care of that.
         MemoryObject(const cl_mem identifier, const bool increment_reference_count);

      private:
         // In general, we DO NOT want to deal with multiple kinds of callbacks, so the user data version is converted to a regular callback as soon as possible
         static DestructorCallback make_destructor_callback(const DestructorCallbackWithUserData & callback, void * const user_data);

         // High-level destructor callbacks are stored here. They will be called by a lower-level static function which follows OpenCL's linkage conventions.
         std::vector<DestructorCallback> * internal_callbacks_ptr;
         void add_destructor_callback(const DestructorCallback & callback);
         static void CL_CALLBACK raw_callback(cl_mem memobj, void * actual_callbacks_ptr);

         // These functions manage the life cycle of reference-counted memory objects
         void copy_internal_data(const MemoryObject & source);
         cl_uint reference_count() const { return raw_uint_query(CL_MEM_REFERENCE_COUNT); }
         void retain() const;
         void release();
   };

   // Because there are multiple kinds of memory objects, which inherit from the same base class, it's important to always pass memory objects by reference.
   // The following typedef makes it easier to pass multiple memory object references to a function, using standard STL vectors.
   using ConstMemoryObjectRefVector = std::vector<std::reference_wrapper<const MemoryObject>>;

}

#endif
