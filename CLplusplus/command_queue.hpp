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

#ifndef INCLUDE_CL_PLUSPLUS_COMMAND_QUEUE
#define INCLUDE_CL_PLUSPLUS_COMMAND_QUEUE

#include <vector>

#include <CL/cl.h>

#include "buffer.hpp"
#include "device.hpp"
#include "event.hpp"
#include "memory_object.hpp"

// This code unit provides facilities for handling OpenCL command queues
namespace CLplusplus {

   class CommandQueue {
      public:
         // === BASIC CLASS LIFECYCLE ===

         // Command queues can be created from a valid OpenCL identifier
         CommandQueue(const cl_command_queue identifier, const bool increment_reference_count);

         // Command queues are reference counted using the following functions
         CommandQueue(const CommandQueue & source);
         ~CommandQueue() { release(); }
         CommandQueue & operator=(const CommandQueue & source);

         // === PROPERTIES ===

         // Command queue properties which are supported by the wrapper are directly accessible in a convenient, high-level fashion
         CLplusplus::Device device() const { return Device{raw_value_query<cl_device_id>(CL_QUEUE_DEVICE), true}; }
         cl_command_queue_properties properties() const { return raw_value_query<cl_command_queue_properties>(CL_QUEUE_PROPERTIES); }

         // Unsupported property values may be queried in a lower-level way
         cl_context raw_context_id() const { return raw_value_query<cl_context>(CL_QUEUE_CONTEXT); } // NOTE : Returning a Context here would lead to a circular dependency.
                                                                                                     // WARNING : Beware that trying to use this identifier in a Context can lead to callback memory leaks.

         // And unsupported command queue properties can be queried in a nearly pure OpenCL way, with some common-case usability optimizations
         template<typename ValueType> ValueType raw_value_query(const cl_command_queue_info parameter_name) const {
            ValueType result;
            raw_query(parameter_name, sizeof(ValueType), &result);
            return result;
         }
         size_t raw_query_output_size(const cl_command_queue_info parameter_name) const;
         void raw_query(const cl_command_queue_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size = nullptr) const;

         // === DEVICE COMMANDS ===

         // All device commands can wait for a list of events, which is specified using the following type
         using EventWaitList = std::vector<Event>;

         // Asynchronously or synchronously read from a buffer to host memory
         void enqueue_read_buffer(const Buffer & source_buffer, const size_t offset, const size_t size, void * const destination, const EventWaitList & event_wait_list) const;
         Event enqueued_read_buffer(const Buffer & source_buffer, const size_t offset, const size_t size, void * const destination, const EventWaitList & event_wait_list) const;
         void read_buffer(const Buffer & source_buffer, const size_t offset, const size_t size, void * const destination, const EventWaitList & event_wait_list) const;

         // TODO : Add an interface to ReadBufferRect

         // Asynchronously write from host memory to a buffer, possibly waiting until the host buffer is safe to modify again to before returning
         // WARNING: This does NOT mean synchronously waiting for a device write to complete. Memory may still be in flight to device memory after the end of this command.
         void enqueue_write_buffer(const void * const source, const bool wait_for_availability, const Buffer & dest_buffer, const size_t offset, const size_t size, const EventWaitList & event_wait_list) const;
         Event enqueued_write_buffer(const void * const source, const bool wait_for_availability, const Buffer & dest_buffer, const size_t offset, const size_t size, const EventWaitList & event_wait_list) const;

         // TODO : Add an interface to WriteBufferRect

         // TODO : Add an interface to CopyBuffer and CopyBufferRect

         // TODO : Add an interface to FillBuffer

         // TODO : Add an interface to MapBuffer

         // Asynchronously unmap a previously mapped memory object
         void enqueue_unmap_mem_object(const MemoryObject & memobj, void * const mapped_ptr, const EventWaitList & event_wait_list) const;
         Event enqueued_unmap_mem_object(const MemoryObject & memobj, void * const mapped_ptr, const EventWaitList & event_wait_list) const;

         // Asynchronously migrate memory objects to the device represented by this command queue
         void enqueue_migrate_mem_objects(const ConstMemoryObjectRefVector & mem_objects, const cl_mem_migration_flags flags, const EventWaitList & event_wait_list) const;
         Event enqueued_migrate_mem_objects(const ConstMemoryObjectRefVector & mem_objects, const cl_mem_migration_flags flags, const EventWaitList & event_wait_list) const;

         // TODO : Add images, kernel execution, etc.

         // === SYNCHRONIZATION ===

         // In an OpenCL command queue, one can wait for some global command-related events
         void flush() const;  // Wait for all previously issued commands to be sent to the device
         void finish() const; // Wait for all previously issued commands to finish execution

         // === RAW OPENCL ID ===

         // Finally, if the need arises, one can directly access the command queue identifier in order to perform raw OpenCL operations.
         // WARNING : Be very careful when you do this, as such raw identifiers will NOT be taken into account during reference counting !
         cl_command_queue raw_identifier() const { return internal_id; }

      private:
         // This is the internal identifier that represents our command queue
         cl_command_queue internal_id;

         // These are the raw OpenCL calls that higher-level functions make
         void raw_read_buffer(const Buffer & source_buffer, const size_t offset, const size_t size, void * const destination, const bool synchronous_read, const EventWaitList & event_wait_list, cl_event * event) const;
         void raw_write_buffer(const void * const source, const bool wait_for_availability, const Buffer & dest_buffer, const size_t offset, const size_t size, const EventWaitList & event_wait_list, cl_event * event) const;
         void raw_unmap_mem_object(const MemoryObject & memobj, void * const mapped_ptr, const EventWaitList & event_wait_list, cl_event * event) const;
         void raw_migrate_mem_objects(const ConstMemoryObjectRefVector & mem_objects, const cl_mem_migration_flags flags, const EventWaitList & event_wait_list, cl_event * event) const;

         // These functions manage the life cycle of reference-counted command queues
         cl_uint reference_count() const { return raw_value_query<cl_uint>(CL_QUEUE_REFERENCE_COUNT); }
         void retain() const;
         void release();
   };

}

#endif
