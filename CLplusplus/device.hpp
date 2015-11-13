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

#ifndef INCLUDE_CL_PLUSPLUS_DEVICE
#define INCLUDE_CL_PLUSPLUS_DEVICE

#include <functional>
#include <string>
#include <vector>

#include <CL/cl.h>

#include "extensions.hpp"
#include "profile.hpp"
#include "property_list.hpp"
#include "version.hpp"

// This code unit provides facilities for handling OpenCL devices
namespace CLplusplus {

   // When an OpenCL device is partitioned into subdevices, some partitioning properties must be specified.
   // For this, OpenCL uses zero-terminated lists, which are relatively impractical to parse and a common source of security issues.
   // We propose an higher-level abstraction, see details in property_list.hpp
   using PartitionProperties = CLplusplus::PropertyList<cl_device_partition_property>;

   // This class represents an OpenCL device that can be queried in a high-level way
   class Device {
      public:
         // We need a valid OpenCL device ID in order to initialize this class
         Device(const cl_device_id identifier, const bool increment_reference_count);

         // Sub-devices are reference counted using the following functions
         Device(const Device & source);
         ~Device();
         Device & operator=(const Device & source);

         // Device properties which are supported by the wrapper are directly accessible in a convenient, high-level fashion
         cl_device_type type() const { return raw_value_query<cl_device_type>(CL_DEVICE_TYPE); }
         cl_uint vendor_id() const { return raw_uint_query(CL_DEVICE_VENDOR_ID); }

         cl_uint max_compute_units() const { return raw_uint_query(CL_DEVICE_MAX_COMPUTE_UNITS); }
         cl_uint max_work_item_dimensions() const { return raw_uint_query(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS); }
         std::vector<size_t> max_work_item_sizes() const;
         size_t max_work_group_size() const { return raw_size_query(CL_DEVICE_MAX_WORK_GROUP_SIZE); }

         cl_uint preferred_vector_width_char() const { return raw_uint_query(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR); }
         cl_uint preferred_vector_width_short() const { return raw_uint_query(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT); }
         cl_uint preferred_vector_width_int() const { return raw_uint_query(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT); }
         cl_uint preferred_vector_width_long() const { return raw_uint_query(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG); }
         cl_uint preferred_vector_width_float() const { return raw_uint_query(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT); }
         cl_uint preferred_vector_width_double() const { return raw_uint_query(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE); }
         cl_uint preferred_vector_width_half() const { return raw_uint_query(CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF); }

         cl_uint native_vector_width_char() const { return raw_uint_query(CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR); }
         cl_uint native_vector_width_short() const { return raw_uint_query(CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT); }
         cl_uint native_vector_width_int() const { return raw_uint_query(CL_DEVICE_NATIVE_VECTOR_WIDTH_INT); }
         cl_uint native_vector_width_long() const { return raw_uint_query(CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG); }
         cl_uint native_vector_width_float() const { return raw_uint_query(CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT); }
         cl_uint native_vector_width_double() const { return raw_uint_query(CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE); }
         cl_uint native_vector_width_half() const { return raw_uint_query(CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF); }

         cl_uint max_clock_frequency() const { return raw_uint_query(CL_DEVICE_MAX_CLOCK_FREQUENCY); }

         cl_uint address_bits() const { return raw_uint_query(CL_DEVICE_ADDRESS_BITS); }

         cl_ulong max_mem_alloc_size() const { return raw_ulong_query(CL_DEVICE_MAX_MEM_ALLOC_SIZE); }

         bool image_support() const { return raw_bool_query(CL_DEVICE_IMAGE_SUPPORT); }
         cl_uint max_read_image_args() const { return raw_uint_query(CL_DEVICE_MAX_READ_IMAGE_ARGS); }
         cl_uint max_write_image_args() const { return raw_uint_query(CL_DEVICE_MAX_WRITE_IMAGE_ARGS); }
         size_t image2d_max_width() const { return raw_size_query(CL_DEVICE_IMAGE2D_MAX_WIDTH); }
         size_t image2d_max_height() const { return raw_size_query(CL_DEVICE_IMAGE2D_MAX_HEIGHT); }
         size_t image3d_max_width() const { return raw_size_query(CL_DEVICE_IMAGE3D_MAX_WIDTH); }
         size_t image3d_max_height() const { return raw_size_query(CL_DEVICE_IMAGE3D_MAX_HEIGHT); }
         size_t image3d_max_depth() const { return raw_size_query(CL_DEVICE_IMAGE3D_MAX_DEPTH); }
         size_t image_max_buffer_size() const { return raw_size_query(CL_DEVICE_IMAGE_MAX_BUFFER_SIZE); }
         size_t image_max_array_size() const { return raw_size_query(CL_DEVICE_IMAGE_MAX_ARRAY_SIZE); }
         cl_uint max_samplers() const { return raw_uint_query(CL_DEVICE_MAX_SAMPLERS); }

         size_t max_parameter_size() const { return raw_size_query(CL_DEVICE_MAX_PARAMETER_SIZE); }

         cl_uint mem_base_addr_align() const { return raw_uint_query(CL_DEVICE_MEM_BASE_ADDR_ALIGN); }

         cl_device_fp_config single_fp_config() const { return raw_value_query<cl_device_fp_config>(CL_DEVICE_SINGLE_FP_CONFIG); }
         cl_device_fp_config double_fp_config() const { return raw_value_query<cl_device_fp_config>(CL_DEVICE_DOUBLE_FP_CONFIG); }

         cl_device_mem_cache_type global_mem_cache_type() const { return raw_value_query<cl_device_mem_cache_type>(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE); }
         cl_uint global_mem_cacheline_size() const { return raw_uint_query(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE); }
         cl_ulong global_mem_cache_size() const { return raw_ulong_query(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE); }
         
         cl_ulong global_mem_size() const { return raw_ulong_query(CL_DEVICE_GLOBAL_MEM_SIZE); }

         cl_ulong max_constant_buffer_size() const { return raw_ulong_query(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE); }
         cl_uint max_constant_args() const { return raw_uint_query(CL_DEVICE_MAX_CONSTANT_ARGS); }

         cl_device_local_mem_type local_mem_type() const { return raw_value_query<cl_device_local_mem_type>(CL_DEVICE_LOCAL_MEM_TYPE); }
         cl_ulong local_mem_size() const { return raw_ulong_query(CL_DEVICE_LOCAL_MEM_SIZE); }

         bool error_correction_support() const { return raw_bool_query(CL_DEVICE_ERROR_CORRECTION_SUPPORT); }

         bool unified_memory() const { return raw_bool_query(CL_DEVICE_HOST_UNIFIED_MEMORY); }

         size_t profiling_timer_resolution() const { return raw_size_query(CL_DEVICE_PROFILING_TIMER_RESOLUTION); }

         bool endian_little() const { return raw_bool_query(CL_DEVICE_ENDIAN_LITTLE); }

         bool available() const { return raw_bool_query(CL_DEVICE_AVAILABLE); }

         bool compiler_available() const { return raw_bool_query(CL_DEVICE_COMPILER_AVAILABLE); }
         bool linker_available() const { return raw_bool_query(CL_DEVICE_LINKER_AVAILABLE); }

         cl_device_exec_capabilities execution_capabilities() const { return raw_value_query<cl_device_exec_capabilities>(CL_DEVICE_EXECUTION_CAPABILITIES); }

         cl_command_queue_properties queue_properties() const { return raw_value_query<cl_command_queue_properties>(CL_DEVICE_QUEUE_PROPERTIES); }

         std::vector<std::string> built_in_kernels() const { return decode_opencl_list(raw_string_query(CL_DEVICE_BUILT_IN_KERNELS), ';'); }

         // NOTE : It is not possible to use a Platform object here, as that would create a recursive dependency.
         cl_platform_id raw_platform_id() const { return raw_value_query<cl_platform_id>(CL_DEVICE_PLATFORM); }

         std::string name() const { return raw_string_query(CL_DEVICE_NAME); }
         std::string vendor() const { return raw_string_query(CL_DEVICE_VENDOR); }
         CLplusplus::Version driver_version() const { return decode_driver_version_string(raw_string_query(CL_DRIVER_VERSION)); }
         CLplusplus::Profile profile() const { return decode_profile_string(raw_profile_string()); }
         CLplusplus::Version version() const { return decode_opencl_version_string(raw_string_query(CL_DEVICE_VERSION)); }
         CLplusplus::Version opencl_c_version() const { return decode_opencl_c_version_string(raw_string_query(CL_DEVICE_OPENCL_C_VERSION)); }
         CLplusplus::ExtensionList extensions() const { return ExtensionList{raw_string_query(CL_DEVICE_EXTENSIONS)}; }

         size_t printf_buffer_size() const { return raw_size_query(CL_DEVICE_PRINTF_BUFFER_SIZE); }

         bool preferred_interop_user_sync() const { return raw_bool_query(CL_DEVICE_PREFERRED_INTEROP_USER_SYNC); }

         bool has_parent_device() const { return (raw_parent_device() != NULL); }
         CLplusplus::Device parent_device() const { return Device{raw_parent_device(), true}; }
         
         cl_uint partition_max_sub_devices() const { return raw_uint_query(CL_DEVICE_PARTITION_MAX_SUB_DEVICES); }
         bool supports_partitioning() const { return (partition_max_sub_devices() > 1); }
         std::vector<cl_device_partition_property> partition_properties() const;
         cl_device_affinity_domain partition_affinity_domain() const { return raw_value_query<cl_device_affinity_domain>(CL_DEVICE_PARTITION_AFFINITY_DOMAIN); }

         PartitionProperties partition_type() const;

         // It is possible to partition some devices into subdevices. For this purpose, we wrap clCreateSubDevices into a cleaner interface.
         std::vector<Device> create_sub_devices(PartitionProperties & properties);

         // Wrapper-unsupported property values may be queried in a lower-level way
         std::string raw_profile_string() const { return raw_string_query(CL_DEVICE_PROFILE); }
         cl_device_id raw_parent_device() const { return raw_value_query<cl_device_id>(CL_DEVICE_PARENT_DEVICE); }

         // And fully unsupported device properties can be queried in a nearly pure OpenCL way, with some common-case usability optimizations
         std::string raw_string_query(const cl_platform_info parameter_name) const;

         template<typename ValueType> ValueType raw_value_query(const cl_device_info parameter_name) const {
            ValueType result;
            raw_query(parameter_name, sizeof(ValueType), &result);
            return result;
         }

         cl_uint raw_uint_query(const cl_device_info parameter_name) const { return raw_value_query<cl_uint>(parameter_name); }
         cl_ulong raw_ulong_query(const cl_device_info parameter_name) const { return raw_value_query<cl_ulong>(parameter_name); }
         size_t raw_size_query(const cl_device_info parameter_name) const { return raw_value_query<size_t>(parameter_name); }
         bool raw_bool_query(const cl_device_info parameter_name) const { return (raw_value_query<cl_bool>(parameter_name) == CL_TRUE); }

         size_t raw_query_output_size(const cl_device_info parameter_name) const;
         void raw_query(const cl_device_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size = nullptr) const;

         // Finally, if the need arises, one can directly access the device identifier in order to perform raw OpenCL operations.
         // WARNING : Be very careful when you do this with subdevices, as such identifiers will NOT be taken into account during device reference counting !
         cl_device_id raw_device_id() const { return internal_id; }

      private:
         cl_device_id internal_id;

         // The following can be used to manage subdevice reference counting
         cl_uint reference_count() const { return raw_uint_query(CL_DEVICE_REFERENCE_COUNT); }
         void retain_device() const;
         void release_device() const;

   };

   // This type is used by code that needs to filter devices according to some criteria
   using DevicePredicate = std::function<bool(const Device &)>;
   
}

#endif
