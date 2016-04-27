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

#include "buffer.hpp"
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
         #ifdef CL_VERSION_1_2
         std::vector<std::string> attributes() const { return decode_opencl_list(raw_string_query(CL_KERNEL_ATTRIBUTES), ' '); }
         #endif

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
         #ifdef CL_VERSION_1_2
         std::array<size_t, 3> global_work_size(const Device & device) const { return raw_work_group_size3_query(device, CL_KERNEL_GLOBAL_WORK_SIZE); }
         #endif
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

         #ifdef CL_VERSION_1_2
         // --- Kernel argument properties ---  (NOTE : These are only available in very specific circumstances, and thus to be used with caution in production code)

         // Argument properties which are supported by the wrapper are directly accessible in a convenient, high-level fashion
         cl_kernel_arg_address_qualifier arg_address_qualifier(const cl_uint arg_indx) const { return raw_argument_value_query<cl_kernel_arg_address_qualifier>(arg_indx, CL_KERNEL_ARG_ADDRESS_QUALIFIER); }
         cl_kernel_arg_access_qualifier arg_access_qualifier(const cl_uint arg_indx) const { return raw_argument_value_query<cl_kernel_arg_address_qualifier>(arg_indx, CL_KERNEL_ARG_ACCESS_QUALIFIER); }
         std::string arg_type_name(const cl_uint arg_indx) const { return raw_argument_string_query(arg_indx, CL_KERNEL_ARG_TYPE_NAME); }
         cl_kernel_arg_type_qualifier arg_type_qualifier(const cl_uint arg_indx) const { return raw_argument_value_query<cl_kernel_arg_type_qualifier>(arg_indx, CL_KERNEL_ARG_TYPE_QUALIFIER); }
         std::string arg_name(const cl_uint arg_indx) const { return raw_argument_string_query(arg_indx, CL_KERNEL_ARG_NAME); }

         // And fully unsupported argument properties can be queried in a nearly pure OpenCL way, with some common-case usability optimizations
         std::string raw_argument_string_query(const cl_uint arg_indx, const cl_kernel_arg_info parameter_name) const;

         template<typename ValueType> ValueType raw_argument_value_query(const cl_uint arg_indx, const cl_kernel_arg_info parameter_name) const {
            ValueType result;
            raw_argument_query(arg_indx, parameter_name, sizeof(ValueType), &result);
            return result;
         }

         size_t raw_argument_query_output_size(const cl_uint arg_indx, const cl_kernel_arg_info parameter_name) const;
         void raw_argument_query(const cl_uint arg_indx, const cl_kernel_arg_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size = nullptr) const;
         #endif

         // === KERNEL ARGUMENT SETUP ===

         // Argument values which are supported by the wrapper may be set in a convenient, high-level fashion
         void set_buffer_argument(const cl_uint arg_index, const Buffer * arg_value) const;
         void set_local_argument(const cl_uint arg_index, const size_t arg_size) const { raw_set_argument(arg_index, arg_size, nullptr); }

         // TODO : Add support for image and sampler arguments, once available

         void set_char_argument(const cl_uint arg_index, const cl_char arg_value) const { raw_set_value_argument<cl_char>(arg_index, arg_value); }
         void set_uchar_argument(const cl_uint arg_index, const cl_uchar arg_value) const { raw_set_value_argument<cl_uchar>(arg_index, arg_value); }
         void set_short_argument(const cl_uint arg_index, const cl_short arg_value) const { raw_set_value_argument<cl_short>(arg_index, arg_value); }
         void set_ushort_argument(const cl_uint arg_index, const cl_ushort arg_value) const { raw_set_value_argument<cl_ushort>(arg_index, arg_value); }
         void set_int_argument(const cl_uint arg_index, const cl_int arg_value) const { raw_set_value_argument<cl_int>(arg_index, arg_value); }
         void set_uint_argument(const cl_uint arg_index, const cl_uint arg_value) const { raw_set_value_argument<cl_uint>(arg_index, arg_value); }
         void set_long_argument(const cl_uint arg_index, const cl_long arg_value) const { raw_set_value_argument<cl_long>(arg_index, arg_value); }
         void set_ulong_argument(const cl_uint arg_index, const cl_ulong arg_value) const { raw_set_value_argument<cl_ulong>(arg_index, arg_value); }

         void set_float_argument(const cl_uint arg_index, const cl_float arg_value) const { raw_set_value_argument<cl_float>(arg_index, arg_value); }
         void set_double_argument(const cl_uint arg_index, const cl_double arg_value) const { raw_set_value_argument<cl_double>(arg_index, arg_value); }
         void set_half_argument(const cl_uint arg_index, const cl_half arg_value) const { raw_set_value_argument<cl_half>(arg_index, arg_value); }

         // And unsupported argument values may be set in a nearly pure OpenCL way, with some common-case usability optimizations
         template<typename ValueType> void raw_set_value_argument(const cl_uint arg_index, const ValueType & value) const { raw_set_argument(arg_index, sizeof(ValueType), static_cast<const void *>(&value)); }

         void raw_set_argument(const cl_uint arg_index, const size_t arg_size, const void * const arg_value) const;

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
