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

#include <CL/cl.h>

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

         // TODO : Implement all kernel object queries

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
