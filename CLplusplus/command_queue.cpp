#include "command_queue.hpp"
#include "common.hpp"

namespace CLplusplus {

   CommandQueue::CommandQueue(const cl_command_queue identifier, const bool increment_reference_count) :
      internal_id{identifier}
   {
      // Handle invalid command queue IDs
      if(internal_id == NULL) throw InvalidArgument();

      // Unless asked not to do so, increment the command queue's reference count
      if(increment_reference_count) retain();
   }

   CommandQueue::CommandQueue(const CommandQueue & source) :
      internal_id{source.internal_id}
   {
      // Whenever a copy of a reference-counted command queue is made, its reference count should be incremented
      retain();
   }

   CommandQueue & CommandQueue::operator=(const CommandQueue & source) {
      // Reference count considerations also apply to copy assignment operator
      internal_id = source.internal_id;
      retain();
      return *this;
   }

   size_t CommandQueue::raw_query_output_size(cl_command_queue_info parameter_name) const {
      size_t result;
      raw_query(parameter_name, 0, nullptr, &result);
      return result;
   }

   void CommandQueue::raw_query(cl_command_queue_info parameter_name, size_t output_storage_size, void * output_storage, size_t * actual_output_size) const {
      throw_if_failed(clGetCommandQueueInfo(internal_id, parameter_name, output_storage_size, output_storage, actual_output_size));
   }

   void CommandQueue::enqueue_unmap_mem_object(const MemoryObject & memobj, void * const mapped_ptr, const std::vector<Event> event_wait_list) const {
      raw_unmap_mem_object(memobj, mapped_ptr, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_unmap_mem_object(const MemoryObject & memobj, void * const mapped_ptr, const std::vector<Event> event_wait_list) const {
      cl_event event_id;
      raw_unmap_mem_object(memobj, mapped_ptr, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_migrate_mem_objects(const std::vector<MemoryObject> & mem_objects, const cl_mem_migration_flags flags, const std::vector<Event> event_wait_list) const {
      raw_migrate_mem_objects(mem_objects, flags, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_migrate_mem_objects(const std::vector<MemoryObject> & mem_objects, const cl_mem_migration_flags flags, const std::vector<Event> event_wait_list) const {
      cl_event event_id;
      raw_migrate_mem_objects(mem_objects, flags, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::flush() const {
      throw_if_failed(clFlush(internal_id));
   }

   void CommandQueue::finish() const {
      throw_if_failed(clFinish(internal_id));
   }

   void CommandQueue::raw_unmap_mem_object(const MemoryObject & memobj, void * const mapped_ptr, const std::vector<Event> event_wait_list, cl_event * event) const {
      const auto num_events = event_wait_list.size();
      if(num_events == 0) {
         // If we are waiting for no event, null out the event wait list in final call
         throw_if_failed(clEnqueueUnmapMemObject(internal_id, memobj.raw_identifier(), mapped_ptr, 0, nullptr, event));
      } else {
         // Convert the event wait list to its OpenCL representation before calling
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         throw_if_failed(clEnqueueUnmapMemObject(internal_id, memobj.raw_identifier(), mapped_ptr, num_events, raw_event_ids, event));
      }
   }

   void CommandQueue::raw_migrate_mem_objects(const std::vector<MemoryObject> & mem_objects, const cl_mem_migration_flags flags, const std::vector<Event> event_wait_list, cl_event * event) const {
      // Convert the memory object list to its OpenCL representation
      const auto num_objects = mem_objects.size();
      cl_mem raw_object_ids[num_objects];
      for(size_t i = 0; i < num_objects; ++i) raw_object_ids[i] = mem_objects[i].raw_identifier();

      // Call for the migration of memory objects
      const auto num_events = event_wait_list.size();
      if(num_events == 0) {
         // If we are waiting for no event, null out the event wait list in final call
         throw_if_failed(clEnqueueMigrateMemObjects(internal_id, num_objects, raw_object_ids, flags, 0, nullptr, event));
      } else {
         // Convert the event wait list to its OpenCL representation before calling
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         throw_if_failed(clEnqueueMigrateMemObjects(internal_id, num_objects, raw_object_ids, flags, num_events, raw_event_ids, event));
      }
   }

   void CommandQueue::retain() const {
      throw_if_failed(clRetainCommandQueue(internal_id));
   }

   void CommandQueue::release() {
      throw_if_failed(clReleaseCommandQueue(internal_id));
   }

}
