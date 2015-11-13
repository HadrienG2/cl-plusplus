#include "command_queue.hpp"
#include "common.hpp"

namespace CLplusplus {

   CommandQueue::CommandQueue(const cl_command_queue identifier, const bool increment_reference_count) :
      internal_id{identifier}
   {
      // Handle invalid command queue IDs
      if(internal_id == NULL) throw InvalidArgument();

      // Unless asked not to do so, increment the command queue's reference count
      if(increment_reference_count) retain_command_queue();
   }

   CommandQueue::CommandQueue(const CommandQueue & source) {
      // Whenever a copy of a reference-counted command queue is made, its reference count should be incremented
      internal_id = source.internal_id;
      retain_command_queue();
   }

   CommandQueue::~CommandQueue() {
      // Decrement command queue reference count, possibly causing its liberation
      release_command_queue();
   }

   CommandQueue & CommandQueue::operator=(const CommandQueue & source) {
      // Reference count considerations also apply to copy assignment operator
      internal_id = source.internal_id;
      retain_command_queue();
      return *this;
   }

   void CommandQueue::flush() const {
      throw_if_failed(clFlush(internal_id));
   }

   void CommandQueue::finish() const {
      throw_if_failed(clFinish(internal_id));
   }

   size_t CommandQueue::raw_query_output_size(cl_command_queue_info parameter_name) const {
      size_t result;
      raw_query(parameter_name, 0, nullptr, &result);
      return result;
   }

   void CommandQueue::raw_query(cl_command_queue_info parameter_name, size_t output_storage_size, void * output_storage, size_t * actual_output_size) const {
      throw_if_failed(clGetCommandQueueInfo(internal_id, parameter_name, output_storage_size, output_storage, actual_output_size));
   }

   void CommandQueue::retain_command_queue() const {
      throw_if_failed(clRetainCommandQueue(internal_id));
   }

   void CommandQueue::release_command_queue() const {
      throw_if_failed(clReleaseCommandQueue(internal_id));
   }

}
