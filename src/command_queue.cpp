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

   Event CommandQueue::enqueued_read_buffer(const Buffer & source_buffer, const size_t source_offset, void * const destination, const size_t size, const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_read_buffer(source_buffer, source_offset, destination, size, false, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_read_buffer(const Buffer & source_buffer, const size_t source_offset,
                                          void * const destination,
                                          const size_t size,
                                          const EventWaitList & event_wait_list) const {
      raw_read_buffer(source_buffer, source_offset, destination, size, false, event_wait_list, nullptr);
   }

   void CommandQueue::read_buffer(const Buffer & source_buffer, const size_t source_offset,
                                  void * const destination,
                                  const size_t size,
                                  const EventWaitList & event_wait_list) const {
      raw_read_buffer(source_buffer, source_offset, destination, size, true, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_read_buffer_rect_2d(const Buffer & source_buffer, const std::array<size_t, 2> source_offset, const size_t source_row_pitch,
                                                    void * const destination, const std::array<size_t, 2> dest_offset, const size_t dest_row_pitch,
                                                    const std::array<size_t, 2> size, const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_read_buffer_rect_2d(source_buffer, source_offset, source_row_pitch, destination, dest_offset, dest_row_pitch, size, false, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_read_buffer_rect_2d(const Buffer & source_buffer, const std::array<size_t, 2> source_offset, const size_t source_row_pitch,
                                                  void * const destination, const std::array<size_t, 2> dest_offset, const size_t dest_row_pitch,
                                                  const std::array<size_t, 2> size, const EventWaitList & event_wait_list) const {
      raw_read_buffer_rect_2d(source_buffer, source_offset, source_row_pitch, destination, dest_offset, dest_row_pitch, size, false, event_wait_list, nullptr);
   }

   void CommandQueue::read_buffer_rect_2d(const Buffer & source_buffer, const std::array<size_t, 2> source_offset, const size_t source_row_pitch,
                                          void * const destination, const std::array<size_t, 2> dest_offset, const size_t dest_row_pitch,
                                          const std::array<size_t, 2> size, const EventWaitList & event_wait_list) const {
      raw_read_buffer_rect_2d(source_buffer, source_offset, source_row_pitch, destination, dest_offset, dest_row_pitch, size, true, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_read_buffer_rect_3d(const Buffer & source_buffer, const std::array<size_t, 3> source_offset, const std::array<size_t, 2> source_pitch,
                                                    void * const destination, const std::array<size_t, 3> dest_offset, const std::array<size_t, 2> dest_pitch,
                                                    const std::array<size_t, 3> size, const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_read_buffer_rect_3d(source_buffer, source_offset, source_pitch, destination, dest_offset, dest_pitch, size, false, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_read_buffer_rect_3d(const Buffer & source_buffer, const std::array<size_t, 3> source_offset, const std::array<size_t, 2> source_pitch,
                                                  void * const destination, const std::array<size_t, 3> dest_offset, const std::array<size_t, 2> dest_pitch,
                                                  const std::array<size_t, 3> size, const EventWaitList & event_wait_list) const {
      raw_read_buffer_rect_3d(source_buffer, source_offset, source_pitch, destination, dest_offset, dest_pitch, size, false, event_wait_list, nullptr);
   }

   void CommandQueue::read_buffer_rect_3d(const Buffer & source_buffer, const std::array<size_t, 3> source_offset, const std::array<size_t, 2> source_pitch,
                                          void * const destination, const std::array<size_t, 3> dest_offset, const std::array<size_t, 2> dest_pitch,
                                          const std::array<size_t, 3> size, const EventWaitList & event_wait_list) const {
      raw_read_buffer_rect_3d(source_buffer, source_offset, source_pitch, destination, dest_offset, dest_pitch, size, true, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_write_buffer(const void * const source, const bool wait_for_availability,
                                             const Buffer & dest_buffer, const size_t dest_offset,
                                             const size_t size,
                                             const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_write_buffer(source, wait_for_availability, dest_buffer, dest_offset, size, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_write_buffer(const void * const source, const bool wait_for_availability,
                                           const Buffer & dest_buffer, const size_t dest_offset,
                                           const size_t size,
                                           const EventWaitList & event_wait_list) const {
      raw_write_buffer(source, wait_for_availability, dest_buffer, dest_offset, size, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_write_buffer_rect_2d(const void * const source, const std::array<size_t, 2> source_offset, const size_t source_row_pitch, const bool wait_for_availability,
                                                     const Buffer & dest_buffer, const std::array<size_t, 2> dest_offset, const size_t dest_row_pitch,
                                                     const std::array<size_t, 2> size, const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_write_buffer_rect_2d(source, source_offset, source_row_pitch, wait_for_availability, dest_buffer, dest_offset, dest_row_pitch, size, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_write_buffer_rect_2d(const void * const source, const std::array<size_t, 2> source_offset, const size_t source_row_pitch, const bool wait_for_availability,
                                                   const Buffer & dest_buffer, const std::array<size_t, 2> dest_offset, const size_t dest_row_pitch,
                                                   const std::array<size_t, 2> size, const EventWaitList & event_wait_list) const {
      raw_write_buffer_rect_2d(source, source_offset, source_row_pitch, wait_for_availability, dest_buffer, dest_offset, dest_row_pitch, size, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_write_buffer_rect_3d(const void * const source, const std::array<size_t, 3> source_offset, const std::array<size_t, 2> source_pitch, const bool wait_for_availability,
                                                     const Buffer & dest_buffer, const std::array<size_t, 3> dest_offset, const std::array<size_t, 2> dest_pitch,
                                                     const std::array<size_t, 3> size, const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_write_buffer_rect_3d(source, source_offset, source_pitch, wait_for_availability, dest_buffer, dest_offset, dest_pitch, size, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_write_buffer_rect_3d(const void * const source, const std::array<size_t, 3> source_offset, const std::array<size_t, 2> source_pitch, const bool wait_for_availability,
                                                   const Buffer & dest_buffer, const std::array<size_t, 3> dest_offset, const std::array<size_t, 2> dest_pitch,
                                                   const std::array<size_t, 3> size, const EventWaitList & event_wait_list) const {
      raw_write_buffer_rect_3d(source, source_offset, source_pitch, wait_for_availability, dest_buffer, dest_offset, dest_pitch, size, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_copy_buffer(const Buffer & source_buffer, const size_t source_offset, const Buffer & dest_buffer, const size_t dest_offset, const size_t size, const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_copy_buffer(source_buffer, source_offset, dest_buffer, dest_offset, size, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_copy_buffer(const Buffer & source_buffer, const size_t source_offset, const Buffer & dest_buffer, const size_t dest_offset, const size_t size, const EventWaitList & event_wait_list) const {
      raw_copy_buffer(source_buffer, source_offset, dest_buffer, dest_offset, size, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_copy_buffer_rect_2d(const Buffer & source_buffer, const std::array<size_t, 2> source_offset, const size_t source_row_pitch,
                                                    const Buffer & dest_buffer, const std::array<size_t, 2> dest_offset, const size_t dest_row_pitch,
                                                    const std::array<size_t, 2> size, const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_copy_buffer_rect_2d(source_buffer, source_offset, source_row_pitch, dest_buffer, dest_offset, dest_row_pitch, size, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_copy_buffer_rect_2d(const Buffer & source_buffer, const std::array<size_t, 2> source_offset, const size_t source_row_pitch,
                                                  const Buffer & dest_buffer, const std::array<size_t, 2> dest_offset, const size_t dest_row_pitch,
                                                  const std::array<size_t, 2> size, const EventWaitList & event_wait_list) const {
      raw_copy_buffer_rect_2d(source_buffer, source_offset, source_row_pitch, dest_buffer, dest_offset, dest_row_pitch, size, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_copy_buffer_rect_3d(const Buffer & source_buffer, const std::array<size_t, 3> source_offset, const std::array<size_t, 2> source_pitch,
                                                    const Buffer & dest_buffer, const std::array<size_t, 3> dest_offset, const std::array<size_t, 2> dest_pitch,
                                                    const std::array<size_t, 3> size, const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_copy_buffer_rect_3d(source_buffer, source_offset, source_pitch, dest_buffer, dest_offset, dest_pitch, size, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_copy_buffer_rect_3d(const Buffer & source_buffer, const std::array<size_t, 3> source_offset, const std::array<size_t, 2> source_pitch,
                                                  const Buffer & dest_buffer, const std::array<size_t, 3> dest_offset, const std::array<size_t, 2> dest_pitch,
                                                  const std::array<size_t, 3> size, const EventWaitList & event_wait_list) const {
      raw_copy_buffer_rect_3d(source_buffer, source_offset, source_pitch, dest_buffer, dest_offset, dest_pitch, size, event_wait_list, nullptr);
   }

   #ifdef CL_VERSION_1_2
   Event CommandQueue::raw_enqueued_fill_buffer(const void * const pattern, const size_t pattern_size, const Buffer & dest_buffer, const size_t offset, const size_t size, const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_fill_buffer(pattern, pattern_size, dest_buffer, offset, size, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::raw_enqueue_fill_buffer(const void * const pattern, const size_t pattern_size, const Buffer & dest_buffer, const size_t offset, const size_t size, const EventWaitList & event_wait_list) const {
      raw_fill_buffer(pattern, pattern_size, dest_buffer, offset, size, event_wait_list, nullptr);
   }
   #endif

   Event CommandQueue::enqueued_map_buffer(const Buffer & buffer, const size_t offset, const size_t size, const cl_map_flags map_flags, const EventWaitList & event_wait_list, void * & future_result) const {
      cl_event event_id;
      future_result = raw_map_buffer(buffer, offset, size, false, map_flags, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_map_buffer(const Buffer & buffer, const size_t offset, const size_t size, const cl_map_flags map_flags, const EventWaitList & event_wait_list, void * & future_result) const {
      future_result = raw_map_buffer(buffer, offset, size, false, map_flags, event_wait_list, nullptr);
   }

   void * CommandQueue::map_buffer(const Buffer & buffer, const size_t offset, const size_t size, const cl_map_flags map_flags, const EventWaitList & event_wait_list) const {
      return raw_map_buffer(buffer, offset, size, true, map_flags, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_read_image_1d(const Image & source_image, const size_t source_origin,
                                              void * const destination,
                                              const size_t region_length,
                                              const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_read_image_1d(source_image, source_origin, destination, region_length, false, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_read_image_1d(const Image & source_image, const size_t source_origin,
                                            void * const destination,
                                            const size_t region_length,
                                            const EventWaitList & event_wait_list) const {
      raw_read_image_1d(source_image, source_origin, destination, region_length, false, event_wait_list, nullptr);
   }

   void CommandQueue::read_image_1d(const Image & source_image, const size_t source_origin,
                                    void * const destination,
                                    const size_t region_length,
                                    const EventWaitList & event_wait_list) const {
      raw_read_image_1d(source_image, source_origin, destination, region_length, true, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_read_image_2d(const Image & source_image, const std::array<size_t, 2> source_origin,
                                              void * const destination, const size_t dest_row_pitch,
                                              const std::array<size_t, 2> region,
                                              const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_read_image_2d(source_image, source_origin, destination, dest_row_pitch, region, false, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_read_image_2d(const Image & source_image, const std::array<size_t, 2> source_origin,
                                            void * const destination, const size_t dest_row_pitch,
                                            const std::array<size_t, 2> region,
                                            const EventWaitList & event_wait_list) const {
      raw_read_image_2d(source_image, source_origin, destination, dest_row_pitch, region, false, event_wait_list, nullptr);
   }

   void CommandQueue::read_image_2d(const Image & source_image, const std::array<size_t, 2> source_origin,
                                    void * const destination, const size_t dest_row_pitch,
                                    const std::array<size_t, 2> region,
                                    const EventWaitList & event_wait_list) const {
      raw_read_image_2d(source_image, source_origin, destination, dest_row_pitch, region, true, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_read_image_3d(const Image & source_image, const std::array<size_t, 3> source_origin,
                                              void * const destination, const std::array<size_t, 2> dest_pitch,
                                              const std::array<size_t, 3> region,
                                              const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_read_image_3d(source_image, source_origin, destination, dest_pitch, region, false, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_read_image_3d(const Image & source_image, const std::array<size_t, 3> source_origin,
                                              void * const destination, const std::array<size_t, 2> dest_pitch,
                                              const std::array<size_t, 3> region,
                                              const EventWaitList & event_wait_list) const {
      raw_read_image_3d(source_image, source_origin, destination, dest_pitch, region, false, event_wait_list, nullptr);
   }

   void CommandQueue::read_image_3d(const Image & source_image, const std::array<size_t, 3> source_origin,
                                              void * const destination, const std::array<size_t, 2> dest_pitch,
                                              const std::array<size_t, 3> region,
                                              const EventWaitList & event_wait_list) const {
      raw_read_image_3d(source_image, source_origin, destination, dest_pitch, region, true, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_unmap_mem_object(const MemoryObject & memobj, void * const mapped_ptr, const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_unmap_mem_object(memobj, mapped_ptr, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_unmap_mem_object(const MemoryObject & memobj, void * const mapped_ptr, const EventWaitList & event_wait_list) const {
      raw_unmap_mem_object(memobj, mapped_ptr, event_wait_list, nullptr);
   }

   #ifdef CL_VERSION_1_2
   Event CommandQueue::enqueued_migrate_mem_objects(const ConstMemoryObjectRefVector & mem_objects, const cl_mem_migration_flags flags, const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_migrate_mem_objects(mem_objects, flags, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_migrate_mem_objects(const ConstMemoryObjectRefVector & mem_objects, const cl_mem_migration_flags flags, const EventWaitList & event_wait_list) const {
      raw_migrate_mem_objects(mem_objects, flags, event_wait_list, nullptr);
   }
   #endif

   Event CommandQueue::enqueued_1d_range_kernel(const Kernel & kernel,
                                                const size_t global_work_size,
                                                const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_Nd_range_kernel(kernel, 1, nullptr, &global_work_size, nullptr, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_1d_range_kernel(const Kernel & kernel,
                                              const size_t global_work_size,
                                              const EventWaitList & event_wait_list) const {
      raw_Nd_range_kernel(kernel, 1, nullptr, &global_work_size, nullptr, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_1d_range_kernel(const Kernel & kernel,
                                                const size_t global_work_size,
                                                const size_t local_work_size,
                                                const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_Nd_range_kernel(kernel, 1, nullptr, &global_work_size, &local_work_size, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_1d_range_kernel(const Kernel & kernel,
                                              const size_t global_work_size,
                                              const size_t local_work_size,
                                              const EventWaitList & event_wait_list) const {
      raw_Nd_range_kernel(kernel, 1, nullptr, &global_work_size, &local_work_size, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_1d_range_kernel(const Kernel & kernel,
                                                const size_t global_work_offset,
                                                const size_t global_work_size,
                                                const size_t local_work_size,
                                                const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_Nd_range_kernel(kernel, 1, &global_work_offset, &global_work_size, &local_work_size, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_1d_range_kernel(const Kernel & kernel,
                                              const size_t global_work_offset,
                                              const size_t global_work_size,
                                              const size_t local_work_size,
                                              const EventWaitList & event_wait_list) const {
      raw_Nd_range_kernel(kernel, 1, &global_work_offset, &global_work_size, &local_work_size, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_2d_range_kernel(const Kernel & kernel,
                                                const std::array<size_t, 2> global_work_size,
                                                const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_Nd_range_kernel(kernel, 2, nullptr, &(global_work_size[0]), nullptr, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_2d_range_kernel(const Kernel & kernel,
                                              const std::array<size_t, 2> global_work_size,
                                              const EventWaitList & event_wait_list) const {
      raw_Nd_range_kernel(kernel, 2, nullptr, &(global_work_size[0]), nullptr, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_2d_range_kernel(const Kernel & kernel,
                                                const std::array<size_t, 2> global_work_size,
                                                const std::array<size_t, 2> local_work_size,
                                                const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_Nd_range_kernel(kernel, 2, nullptr, &(global_work_size[0]), &(local_work_size[0]), event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_2d_range_kernel(const Kernel & kernel,
                                              const std::array<size_t, 2> global_work_size,
                                              const std::array<size_t, 2> local_work_size,
                                              const EventWaitList & event_wait_list) const {
      raw_Nd_range_kernel(kernel, 2, nullptr, &(global_work_size[0]), &(local_work_size[0]), event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_2d_range_kernel(const Kernel & kernel,
                                                const std::array<size_t, 2> global_work_offset,
                                                const std::array<size_t, 2> global_work_size,
                                                const std::array<size_t, 2> local_work_size,
                                                const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_Nd_range_kernel(kernel, 2, &(global_work_offset[0]), &(global_work_size[0]), &(local_work_size[0]), event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_2d_range_kernel(const Kernel & kernel,
                                              const std::array<size_t, 2> global_work_offset,
                                              const std::array<size_t, 2> global_work_size,
                                              const std::array<size_t, 2> local_work_size,
                                              const EventWaitList & event_wait_list) const {
      raw_Nd_range_kernel(kernel, 2, &(global_work_offset[0]), &(global_work_size[0]), &(local_work_size[0]), event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_3d_range_kernel(const Kernel & kernel,
                                                const std::array<size_t, 3> global_work_size,
                                                const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_Nd_range_kernel(kernel, 3, nullptr, &(global_work_size[0]), nullptr, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_3d_range_kernel(const Kernel & kernel,
                                              const std::array<size_t, 3> global_work_size,
                                              const EventWaitList & event_wait_list) const {
      raw_Nd_range_kernel(kernel, 3, nullptr, &(global_work_size[0]), nullptr, event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_3d_range_kernel(const Kernel & kernel,
                                                const std::array<size_t, 3> global_work_size,
                                                const std::array<size_t, 3> local_work_size,
                                                const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_Nd_range_kernel(kernel, 3, nullptr, &(global_work_size[0]), &(local_work_size[0]), event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_3d_range_kernel(const Kernel & kernel,
                                              const std::array<size_t, 3> global_work_size,
                                              const std::array<size_t, 3> local_work_size,
                                              const EventWaitList & event_wait_list) const {
      raw_Nd_range_kernel(kernel, 3, nullptr, &(global_work_size[0]), &(local_work_size[0]), event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_3d_range_kernel(const Kernel & kernel,
                                                const std::array<size_t, 3> global_work_offset,
                                                const std::array<size_t, 3> global_work_size,
                                                const std::array<size_t, 3> local_work_size,
                                                const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_Nd_range_kernel(kernel, 3, &(global_work_offset[0]), &(global_work_size[0]), &(local_work_size[0]), event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_3d_range_kernel(const Kernel & kernel,
                                              const std::array<size_t, 3> global_work_offset,
                                              const std::array<size_t, 3> global_work_size,
                                              const std::array<size_t, 3> local_work_size,
                                              const EventWaitList & event_wait_list) const {
      raw_Nd_range_kernel(kernel, 3, &(global_work_offset[0]), &(global_work_size[0]), &(local_work_size[0]), event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_task(const Kernel & kernel, const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_task(kernel, event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_task(const Kernel & kernel, const EventWaitList & event_wait_list) const {
      raw_task(kernel, event_wait_list, nullptr);
   }

   #ifdef CL_VERSION_1_2
   Event CommandQueue::enqueued_marker_with_wait_list(const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_marker_with_wait_list(event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_marker_with_wait_list(const EventWaitList & event_wait_list) const {
      raw_marker_with_wait_list(event_wait_list, nullptr);
   }

   Event CommandQueue::enqueued_barrier_with_wait_list(const EventWaitList & event_wait_list) const {
      cl_event event_id;
      raw_barrier_with_wait_list(event_wait_list, &event_id);
      return Event{event_id, false};
   }

   void CommandQueue::enqueue_barrier_with_wait_list(const EventWaitList & event_wait_list) const {
      raw_barrier_with_wait_list(event_wait_list, nullptr);
   }
   #endif

   void CommandQueue::flush() const {
      throw_if_failed(clFlush(internal_id));
   }

   void CommandQueue::finish() const {
      throw_if_failed(clFinish(internal_id));
   }

   void CommandQueue::raw_read_buffer(const Buffer & source_buffer, const size_t offset,
                                      void * const destination,
                                      const size_t size, const bool synchronous_read,
                                      const EventWaitList & event_wait_list, cl_event * const event) const {
      const auto num_events = event_wait_list.size();
      if(num_events == 0) {
         throw_if_failed(clEnqueueReadBuffer(internal_id, source_buffer.raw_identifier(), synchronous_read, offset, size, destination, 0, nullptr, event));
      } else {
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         throw_if_failed(clEnqueueReadBuffer(internal_id, source_buffer.raw_identifier(), synchronous_read, offset, size, destination, num_events, raw_event_ids, event));
      }
   }

   void CommandQueue::raw_read_buffer_rect_2d(const Buffer & source_buffer, const std::array<size_t, 2> source_offset, const size_t source_row_pitch,
                                              void * const destination, const std::array<size_t, 2> dest_offset, const size_t dest_row_pitch,
                                              const std::array<size_t, 2> size, const bool synchronous_read,
                                              const EventWaitList & event_wait_list, cl_event * const event) const {
      // Build a 3D equivalent of our 2D rectangle read request
      const std::array<size_t, 3> source_offset_3d {source_offset[0], source_offset[1], 0};
      const std::array<size_t, 2> source_pitch_3d {source_row_pitch, 0};
      const std::array<size_t, 3> dest_offset_3d {dest_offset[0], dest_offset[1], 0};
      const std::array<size_t, 2> dest_pitch_3d {dest_row_pitch, 0};
      const std::array<size_t, 3> size_3d {size[0], size[1], 1};

      // Run that request
      raw_read_buffer_rect_3d(source_buffer, source_offset_3d, source_pitch_3d, destination, dest_offset_3d, dest_pitch_3d, size_3d, synchronous_read, event_wait_list, event);
   }

   void CommandQueue::raw_read_buffer_rect_3d(const Buffer & source_buffer, const std::array<size_t, 3> source_offset, const std::array<size_t, 2> source_pitch,
                                              void * const destination, const std::array<size_t, 3> dest_offset, const std::array<size_t, 2> dest_pitch,
                                              const std::array<size_t, 3> size, const bool synchronous_read,
                                              const EventWaitList & event_wait_list, cl_event * const event) const {
      const auto num_events = event_wait_list.size();
      if(num_events == 0) {
         throw_if_failed(clEnqueueReadBufferRect(internal_id, source_buffer.raw_identifier(), synchronous_read,
                                                 source_offset.data(), dest_offset.data(), size.data(),
                                                 source_pitch[0], source_pitch[1], dest_pitch[0], dest_pitch[1],
                                                 destination, 0, nullptr, event));
      } else {
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         throw_if_failed(clEnqueueReadBufferRect(internal_id, source_buffer.raw_identifier(), synchronous_read,
                                                 source_offset.data(), dest_offset.data(), size.data(),
                                                 source_pitch[0], source_pitch[1], dest_pitch[0], dest_pitch[1],
                                                 destination, num_events, raw_event_ids, event));
      }
   }

   void CommandQueue::raw_write_buffer(const void * const source, const bool wait_for_availability,
                                       const Buffer & dest_buffer, const size_t offset,
                                       const size_t size,
                                       const EventWaitList & event_wait_list, cl_event * const event) const {
      const auto num_events = event_wait_list.size();
      if(num_events == 0) {
         throw_if_failed(clEnqueueWriteBuffer(internal_id, dest_buffer.raw_identifier(), wait_for_availability, offset, size, source, 0, nullptr, event));
      } else {
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         throw_if_failed(clEnqueueWriteBuffer(internal_id, dest_buffer.raw_identifier(), wait_for_availability, offset, size, source, num_events, raw_event_ids, event));
      }
   }

   void CommandQueue::raw_write_buffer_rect_2d(const void * const source, const std::array<size_t, 2> source_offset, const size_t source_row_pitch, const bool wait_for_availability,
                                               const Buffer & dest_buffer, const std::array<size_t, 2> dest_offset, const size_t dest_row_pitch,
                                               const std::array<size_t, 2> size,
                                               const EventWaitList & event_wait_list, cl_event * const event) const {
      // Build a 3D equivalent of our 2D rectangle write request
      const std::array<size_t, 3> source_offset_3d {source_offset[0], source_offset[1], 0};
      const std::array<size_t, 2> source_pitch_3d {source_row_pitch, 0};
      const std::array<size_t, 3> dest_offset_3d {dest_offset[0], dest_offset[1], 0};
      const std::array<size_t, 2> dest_pitch_3d {dest_row_pitch, 0};
      const std::array<size_t, 3> size_3d {size[0], size[1], 1};

      // Run that request
      raw_write_buffer_rect_3d(source, source_offset_3d, source_pitch_3d, wait_for_availability, dest_buffer, dest_offset_3d, dest_pitch_3d, size_3d, event_wait_list, event);
   }

   void CommandQueue::raw_write_buffer_rect_3d(const void * const source, const std::array<size_t, 3> source_offset, const std::array<size_t, 2> source_pitch, const bool wait_for_availability,
                                               const Buffer & dest_buffer, const std::array<size_t, 3> dest_offset, const std::array<size_t, 2> dest_pitch,
                                               const std::array<size_t, 3> size,
                                               const EventWaitList & event_wait_list, cl_event * const event) const {
      const auto num_events = event_wait_list.size();
      if(num_events == 0) {
         throw_if_failed(clEnqueueWriteBufferRect(internal_id, dest_buffer.raw_identifier(), wait_for_availability,
                                                  source_offset.data(), dest_offset.data(), size.data(),
                                                  source_pitch[0], source_pitch[1], dest_pitch[0], dest_pitch[1],
                                                  source, 0, nullptr, event));
      } else {
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         throw_if_failed(clEnqueueWriteBufferRect(internal_id, dest_buffer.raw_identifier(), wait_for_availability,
                                                  source_offset.data(), dest_offset.data(), size.data(),
                                                  source_pitch[0], source_pitch[1], dest_pitch[0], dest_pitch[1],
                                                  source, num_events, raw_event_ids, event));
      }
   }

   void CommandQueue::raw_copy_buffer(const Buffer & source_buffer, const size_t source_offset,
                                      const Buffer & dest_buffer, const size_t dest_offset,
                                      const size_t size,
                                      const EventWaitList & event_wait_list, cl_event * const event) const {
      const auto num_events = event_wait_list.size();
      if(num_events == 0) {
         throw_if_failed(clEnqueueCopyBuffer(internal_id, source_buffer.raw_identifier(), dest_buffer.raw_identifier(), source_offset, dest_offset, size, 0, nullptr, event));
      } else {
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         throw_if_failed(clEnqueueCopyBuffer(internal_id, source_buffer.raw_identifier(), dest_buffer.raw_identifier(), source_offset, dest_offset, size, num_events, raw_event_ids, event));
      }
   }

   void CommandQueue::raw_copy_buffer_rect_2d(const Buffer & source_buffer, const std::array<size_t, 2> source_offset, const size_t source_row_pitch,
                                              const Buffer & dest_buffer, const std::array<size_t, 2> dest_offset, const size_t dest_row_pitch,
                                              const std::array<size_t, 2> size,
                                              const EventWaitList & event_wait_list, cl_event * const event) const {
      // Build a 3D equivalent of our 2D rectangle copy request
      const std::array<size_t, 3> source_offset_3d {source_offset[0], source_offset[1], 0};
      const std::array<size_t, 2> source_pitch_3d {source_row_pitch, 0};
      const std::array<size_t, 3> dest_offset_3d {dest_offset[0], dest_offset[1], 0};
      const std::array<size_t, 2> dest_pitch_3d {dest_row_pitch, 0};
      const std::array<size_t, 3> size_3d {size[0], size[1], 1};

      // Run that request
      raw_copy_buffer_rect_3d(source_buffer, source_offset_3d, source_pitch_3d, dest_buffer, dest_offset_3d, dest_pitch_3d, size_3d, event_wait_list, event);
   }

   void CommandQueue::raw_copy_buffer_rect_3d(const Buffer & source_buffer, const std::array<size_t, 3> source_offset, const std::array<size_t, 2> source_pitch,
                                              const Buffer & dest_buffer, const std::array<size_t, 3> dest_offset, const std::array<size_t, 2> dest_pitch,
                                              const std::array<size_t, 3> size,
                                              const EventWaitList & event_wait_list, cl_event * const event) const {
      const auto num_events = event_wait_list.size();
      if(num_events == 0) {
         throw_if_failed(clEnqueueCopyBufferRect(internal_id, source_buffer.raw_identifier(), dest_buffer.raw_identifier(),
                                                 source_offset.data(), dest_offset.data(), size.data(),
                                                 source_pitch[0], source_pitch[1], dest_pitch[0], dest_pitch[1],
                                                 0, nullptr, event));
      } else {
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         throw_if_failed(clEnqueueCopyBufferRect(internal_id, source_buffer.raw_identifier(), dest_buffer.raw_identifier(),
                                                 source_offset.data(), dest_offset.data(), size.data(),
                                                 source_pitch[0], source_pitch[1], dest_pitch[0], dest_pitch[1],
                                                 num_events, raw_event_ids, event));
      }
   }

   #ifdef CL_VERSION_1_2
   void CommandQueue::raw_fill_buffer(const void * const pattern, const size_t pattern_size, const Buffer & dest_buffer, const size_t offset, const size_t size, const EventWaitList & event_wait_list, cl_event * const event) const {
      const auto num_events = event_wait_list.size();
      if(num_events == 0) {
         throw_if_failed(clEnqueueFillBuffer(internal_id, dest_buffer.raw_identifier(), pattern, pattern_size, offset, size, 0, nullptr, event));
      } else {
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         throw_if_failed(clEnqueueFillBuffer(internal_id, dest_buffer.raw_identifier(), pattern, pattern_size, offset, size, num_events, raw_event_ids, event));
      }
   }
   #endif

   void * CommandQueue::raw_map_buffer(const Buffer & buffer, const size_t offset, const size_t size, const bool synchronous_map, const cl_map_flags map_flags, const EventWaitList & event_wait_list, cl_event * const event) const {
      // Determine if we are waiting for events, and prepare error code and result storage
      const auto num_events = event_wait_list.size();
      cl_int error_code;
      void * result;

      // Send the raw OpenCL command, with a wait list if needed
      if(num_events == 0) {
         result = clEnqueueMapBuffer(internal_id, buffer.raw_identifier(), synchronous_map, map_flags, offset, size, 0, nullptr, event, &error_code);
      } else {
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         result = clEnqueueMapBuffer(internal_id, buffer.raw_identifier(), synchronous_map, map_flags, offset, size, num_events, raw_event_ids, event, &error_code);
      }

      // Check if an error occured, and throw exceptions accordingly
      throw_if_failed(error_code);
      return result;
   }

   void CommandQueue::raw_read_image_1d(const Image & source_image, const size_t source_origin,
                                        void * const destination,
                                        const size_t region_length, const bool synchronous_read,
                                        const EventWaitList & event_wait_list, cl_event * const event) const {
      // Build a 3D equivalent of our 1D image read request
      const std::array<size_t, 3> source_origin_3d {source_origin, 0, 0};
      const std::array<size_t, 2> dest_pitch_3d {0, 0};
      const std::array<size_t, 3> region_3d {region_length, 1, 1};

      // Run that request
      raw_read_image_3d(source_image, source_origin_3d, destination, dest_pitch_3d, region_3d, synchronous_read, event_wait_list, event);
   }

   void CommandQueue::raw_read_image_2d(const Image & source_image, const std::array<size_t, 2> source_origin,
                                        void * const destination, const size_t dest_row_pitch,
                                        const std::array<size_t, 2> region, const bool synchronous_read,
                                        const EventWaitList & event_wait_list, cl_event * const event) const {
      // Build a 3D equivalent of our 2D image read request
      const std::array<size_t, 3> source_origin_3d {source_origin[0], source_origin[1], 0};
      const std::array<size_t, 2> dest_pitch_3d {dest_row_pitch, 0};
      const std::array<size_t, 3> region_3d {region[0], region[1], 1};

      // Run that request
      raw_read_image_3d(source_image, source_origin_3d, destination, dest_pitch_3d, region_3d, synchronous_read, event_wait_list, event);
   }

   void CommandQueue::raw_read_image_3d(const Image & source_image, const std::array<size_t, 3> source_origin,
                                        void * const destination, const std::array<size_t, 2> dest_pitch,
                                        const std::array<size_t, 3> region, const bool synchronous_read,
                                        const EventWaitList & event_wait_list, cl_event * const event) const {
      const auto num_events = event_wait_list.size();
      if(num_events == 0) {
         throw_if_failed(clEnqueueReadImage(internal_id, source_image.raw_identifier(), synchronous_read,
                                            source_origin.data(), region.data(),
                                            dest_pitch[0], dest_pitch[1],
                                            destination,
                                            0, nullptr, event));
      } else {
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         throw_if_failed(clEnqueueReadImage(internal_id, source_image.raw_identifier(), synchronous_read,
                                            source_origin.data(), region.data(),
                                            dest_pitch[0], dest_pitch[1],
                                            destination,
                                            num_events, raw_event_ids, event));
      }
   }

   void CommandQueue::raw_unmap_mem_object(const MemoryObject & memobj, void * const mapped_ptr, const EventWaitList & event_wait_list, cl_event * const event) const {
      const auto num_events = event_wait_list.size();
      if(num_events == 0) {
         throw_if_failed(clEnqueueUnmapMemObject(internal_id, memobj.raw_identifier(), mapped_ptr, 0, nullptr, event));
      } else {
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         throw_if_failed(clEnqueueUnmapMemObject(internal_id, memobj.raw_identifier(), mapped_ptr, num_events, raw_event_ids, event));
      }
   }

   #ifdef CL_VERSION_1_2
   void CommandQueue::raw_migrate_mem_objects(const ConstMemoryObjectRefVector & mem_objects, const cl_mem_migration_flags flags, const EventWaitList & event_wait_list, cl_event * const event) const {
      // Convert the memory object list to its OpenCL representation
      const auto num_objects = mem_objects.size();
      cl_mem raw_object_ids[num_objects];
      for(size_t i = 0; i < num_objects; ++i) raw_object_ids[i] = mem_objects[i].get().raw_identifier();

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
   #endif

   void CommandQueue::raw_Nd_range_kernel(const Kernel & kernel, const cl_uint work_dim,
                                          const size_t * const global_work_offset,
                                          const size_t * const global_work_size,
                                          const size_t * const local_work_size,
                                          const EventWaitList & event_wait_list, cl_event * const event) const {
      const auto num_events = event_wait_list.size();
      if(num_events == 0) {
         throw_if_failed(clEnqueueNDRangeKernel(internal_id, kernel.raw_identifier(), work_dim,
                                                global_work_offset,
                                                global_work_size,
                                                local_work_size,
                                                0, nullptr, event));
      } else {
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         throw_if_failed(clEnqueueNDRangeKernel(internal_id, kernel.raw_identifier(), work_dim,
                                                global_work_offset,
                                                global_work_size,
                                                local_work_size,
                                                num_events, raw_event_ids, event));
      }
   }

   void CommandQueue::raw_task(const Kernel & kernel, const EventWaitList & event_wait_list, cl_event * const event) const {
      const auto num_events = event_wait_list.size();
      if(num_events == 0) {
         throw_if_failed(clEnqueueTask(internal_id, kernel.raw_identifier(), 0, nullptr, event));
      } else {
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         throw_if_failed(clEnqueueTask(internal_id, kernel.raw_identifier(), num_events, raw_event_ids, event));
      }
   }

   #ifdef CL_VERSION_1_2
   void CommandQueue::raw_marker_with_wait_list(const EventWaitList & event_wait_list, cl_event * const event) const {
      const auto num_events = event_wait_list.size();
      if(num_events == 0) {
         throw_if_failed(clEnqueueMarkerWithWaitList(internal_id, 0, nullptr, event));
      } else {
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         throw_if_failed(clEnqueueMarkerWithWaitList(internal_id, num_events, raw_event_ids, event));
      }
   }

   void CommandQueue::raw_barrier_with_wait_list(const EventWaitList & event_wait_list, cl_event * const event) const {
      const auto num_events = event_wait_list.size();
      if(num_events == 0) {
         throw_if_failed(clEnqueueBarrierWithWaitList(internal_id, 0, nullptr, event));
      } else {
         cl_event raw_event_ids[num_events];
         for(size_t i = 0; i < num_events; ++i) raw_event_ids[i] = event_wait_list[i].raw_identifier();
         throw_if_failed(clEnqueueBarrierWithWaitList(internal_id, num_events, raw_event_ids, event));
      }
   }
   #endif

   void CommandQueue::retain() const {
      throw_if_failed(clRetainCommandQueue(internal_id));
   }

   void CommandQueue::release() {
      throw_if_failed(clReleaseCommandQueue(internal_id));
   }

}
