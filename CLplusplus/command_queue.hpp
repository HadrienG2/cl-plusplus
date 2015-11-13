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

#include <CL/cl.h>

#include "device.hpp"

// This code unit provides facilities for handling OpenCL command queues
namespace CLplusplus {

   class CommandQueue {
      public:
         // Command queues can be create from a valid OpenCL identifier
         CommandQueue(const cl_command_queue identifier, const bool increment_reference_count);

         // Command queues are reference counted using the following functions
         CommandQueue(const CommandQueue & source);
         ~CommandQueue();
         CommandQueue & operator=(const CommandQueue & source);

         // Command queue properties which are supported by the wrapper are directly accessible in a convenient, high-level fashion
         cl_context raw_context_id() const { return raw_value_query<cl_context>(CL_QUEUE_CONTEXT); } // NOTE : We cannot provide a context object here, as that would lead to a circular dependency
         Device device() const { return Device{raw_value_query<cl_device_id>(CL_QUEUE_DEVICE), true}; }
         cl_command_queue_properties properties() const { return raw_value_query<cl_command_queue_properties>(CL_QUEUE_PROPERTIES); }

         // And unsupported command queue properties can be queried in a nearly pure OpenCL way, with some common-case usability optimizations
         template<typename ValueType> ValueType raw_value_query(const cl_command_queue_info parameter_name) const {
            ValueType result;
            raw_query(parameter_name, sizeof(ValueType), &result);
            return result;
         }
         size_t raw_query_output_size(const cl_command_queue_info parameter_name) const;
         void raw_query(const cl_command_queue_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size = nullptr) const;

      private:
         // This is the internal identifier that represents our command queue
         cl_command_queue internal_id;

         // These functions manage the life cycle of reference-counted command queues
         cl_uint reference_count() const { return raw_value_query<cl_uint>(CL_QUEUE_REFERENCE_COUNT); }
         void retain_command_queue() const;
         void release_command_queue() const;
   };

}

#endif
