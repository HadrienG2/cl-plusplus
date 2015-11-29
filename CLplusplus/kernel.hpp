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

#ifndef INCLUDE_CL_PLUSPLUS_KERNEL
#define INCLUDE_CL_PLUSPLUS_KERNEL

#include <array>
#include <string>

#include <CL/cl.h>

#include "device.hpp"

// This code unit provides a high-level way to manage OpenCL kernels
namespace CLplusplus {

   class Kernel {
      public:
         // === BASIC CLASS LIFECYCLE ===

         // Kernel objects can be created from a valid OpenCL identifier
         Kernel(const cl_kernel identifier, const bool increment_reference_count);

         // Kernel objects are reference counted using the following functions
         Kernel(const Kernel & source);
         ~Kernel() { release(); }
         Kernel & operator=(const Kernel & source);

         // === PROPERTIES ===

         // --- Global kernel properties ---

         // Kernel properties which are supported by the wrapper are directly accessible in a convenient, high-level fashion
         std::string function_name() const { return raw_string_query(CL_KERNEL_FUNCTION_NAME); }
         cl_uint num_args() const { return raw_uint_query(CL_KERNEL_NUM_ARGS); }
         std::vector<std::string> attributes() const { return decode_opencl_list(raw_string_query(CL_KERNEL_ATTRIBUTES), ' '); }

         // Unsupported property values may be queried in a lower-level way
         cl_context raw_context_id() const { return raw_value_query<cl_context>(CL_KERNEL_CONTEXT); } // WARNING : Beware that trying to use this identifier in a Context can lead to callback memory leaks.
         cl_program raw_program_id() const { return raw_value_query<cl_program>(CL_KERNEL_PROGRAM); } // WARNING : Beware that trying to use this identifier in a Program can lead to callback memory leaks.

         // And fully unsupported kernel properties can be queried in a nearly pure OpenCL way, with some common-case usability optimizations
         std::string raw_string_query(const cl_kernel_info parameter_name) const;
         cl_uint raw_uint_query(const cl_kernel_info parameter_name) const { return raw_value_query<cl_uint>(parameter_name); }

         template<typename ValueType> ValueType raw_value_query(const cl_kernel_info parameter_name) const {
            ValueType result;
            raw_query(parameter_name, sizeof(ValueType), &result);
            return result;
         }

         size_t raw_query_output_size(const cl_kernel_info parameter_name) const;
         void raw_query(const cl_kernel_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size = nullptr) const;

         // --- Device-specific kernel properties, aka "work-group info" ---

         // Device-specific properties which are supported by the wrapper are directly accessible in a convenient, high-level fashion
         std::array<size_t, 3> global_work_size(const Device & device) const { return raw_work_group_size3_query(device, CL_KERNEL_GLOBAL_WORK_SIZE); }
         size_t work_group_size(const Device & device) const { return raw_work_group_size_query(device, CL_KERNEL_WORK_GROUP_SIZE); }
         std::array<size_t, 3> compile_work_group_size(const Device & device) const { return raw_work_group_size3_query(device, CL_KERNEL_COMPILE_WORK_GROUP_SIZE); }
         cl_ulong local_mem_size(const Device & device) const { return raw_work_group_ulong_query(device, CL_KERNEL_LOCAL_MEM_SIZE); }
         size_t preferred_work_group_size_multiple(const Device & device) const { return raw_work_group_size_query(device, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE); }
         cl_ulong private_mem_size(const Device & device) const { return raw_work_group_ulong_query(device, CL_KERNEL_PRIVATE_MEM_SIZE); }

         // And fully unsupported device-specific properties can be queried in a nearly pure OpenCL way, with some common-case usability optimizations
         std::array<size_t, 3> raw_work_group_size3_query(const Device & device, const cl_kernel_work_group_info parameter_name) const;
         size_t raw_work_group_size_query(const Device & device, const cl_kernel_work_group_info parameter_name) const { return raw_work_group_value_query<size_t>(device, parameter_name); }
         cl_ulong raw_work_group_ulong_query(const Device & device, const cl_kernel_work_group_info parameter_name) const { return raw_work_group_value_query<cl_ulong>(device, parameter_name); }

         template<typename ValueType> ValueType raw_work_group_value_query(const Device & device, const cl_kernel_work_group_info parameter_name) const {
            ValueType result;
            raw_work_group_query(device, parameter_name, sizeof(ValueType), &result);
            return result;
         }

         size_t raw_work_group_query_output_size(const Device & device, const cl_kernel_work_group_info parameter_name) const;
         void raw_work_group_query(const Device & device, const cl_kernel_work_group_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size = nullptr) const;

         // --- TODO : Kernel argument properties ---

         // === KERNEL ARGUMENT SETUP ===

         // TODO : Implement a high-level, user-friendly interface above clSetKernelArg()

         // === RAW OPENCL ID ===

         // Finally, if the need arises, one can directly access the kernel object identifier in order to perform raw OpenCL operations.
         // WARNING : Be very careful when you do this, as such raw identifiers will NOT be taken into account during reference counting !
         cl_kernel raw_identifier() const { return internal_id; }

      private:
         // This is the internal identifier that represents our kernel object
         cl_kernel internal_id;

         // These functions manage the life cycle of reference-counted kernel objects
         cl_uint reference_count() const { return raw_uint_query(CL_KERNEL_REFERENCE_COUNT); }
         void retain() const;
         void release();
   };

}

#endif
