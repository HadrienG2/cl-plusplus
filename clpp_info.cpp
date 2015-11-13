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

#include <iostream>

#include <CL/cl.h>

#include "CLplusplus/common.hpp"
#include "CLplusplus/device.hpp"
#include "CLplusplus/platform.hpp"
#include "CLplusplus/profile.hpp"
#include "CLplusplus/queries.hpp"
#include "CLplusplus/version.hpp"

// Some minimal platform and device parameters are specified here, but most of them are specified below
const CLplusplus::Version target_version = CLplusplus::version_1p2;
const cl_ulong min_mem_alloc_size = 20 * 1024 * 1024;
const cl_ulong min_local_mem_size = 16 * 1024;

// This program implements a cousin of the well-known CLinfo program, illustrating the syntax and capabilities of CLplusplus
int main() {
   // Detect OpenCL platform and device combinations which match our expectations
   const auto filtered_platforms = CLplusplus::get_filtered_devices(
      [](const CLplusplus::Platform & platform) -> bool {
         return (platform.version() >= target_version);                       // Platform OpenCL version is recent enough
      },
      [](const CLplusplus::Device & device) -> bool {
         const bool device_supports_ooe_execution = device.queue_properties() & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
         const auto device_double_config = device.double_fp_config();
         return device.available() &&                                         // Device is available for compute purposes
                (device.version() >= target_version) &&                       // Device OpenCL version is recent enough
                device.endian_little() &&                                     // Device is little-endian
                (device.execution_capabilities() & CL_EXEC_KERNEL) &&         // Device can execute OpenCL kernels
                device_supports_ooe_execution &&                              // Device can execute OpenCL commands out of order
                device.compiler_available() && device.linker_available() &&   // Implementation has an OpenCL C compiler and linker for this device
                (device.max_mem_alloc_size() >= min_mem_alloc_size) &&        // Device accepts large enough global memory allocations
                (device.local_mem_type() == CL_LOCAL) &&                      // Device has local memory support, with dedicated storage
                (device.local_mem_size() >= min_local_mem_size) &&            // Device has a large enough local memory
                (device_double_config != 0) &&                                // Doubles are supported
                ((device_double_config & CL_FP_SOFT_FLOAT) == 0);             // Doubles are not emulated in software
      }
   );

   // Display the basic platform detection result
   if(filtered_platforms.size() > 0) {
      std::cout << filtered_platforms.size() << " suitable OpenCL platform(s) detected" << std::endl;
   } else {
      std::cout << "No suitable OpenCL platform or device detected!" << std::endl;
      std::abort();
   }

   // Investigate each detected OpenCL platform
   for(size_t i = 0; i < filtered_platforms.size(); ++i) {

      // Print some introductory text
      std::cout << std::endl << "=== Investigating platform " << i << " ===" << std::endl << std::endl;
      const auto & platform = filtered_platforms[i].platform;
      const auto & devices = filtered_platforms[i].filtered_devices;
      
      // Detect platform profile
      const auto profile = platform.profile();
      switch(profile) {
         case CLplusplus::Profile::Full:
            std::cout << "Platform implements OpenCL Full Profile" << std::endl;
            break;
         case CLplusplus::Profile::Embedded:
            std::cout << "Platform implements OpenCL Embedded Profile" << std::endl;
            break;
         default:
            std::cout << "Platform implements an unknown OpenCL profile" << std::endl;
      }

      // Detect platform version
      const auto version = platform.version();
      std::cout << "OpenCL version is " << version.major << "." << version.minor << " [" << version.vendor_specific_info << "]" << std::endl;

      // Detect platform name
      std::cout << "Platform name is " << platform.name() << std::endl;

      // Detect vendor name
      std::cout << "Platform vendor is " << platform.vendor() << std::endl;

      // Detect extensions
      const auto extensions = platform.extensions();
      std::cout << "Platform supports " << extensions.size() << " extensions: ";
      for(const auto & extension : extensions) {
         std::cout << extension << " ";
      }
      std::cout << std::endl;

      // Determine how many suitable devices were found on this platform
      std::cout << std::endl << "Platform features " << devices.size() << " suitable device(s)" << std::endl;

      // Investigate each detected OpenCL device
      for(size_t j = 0; j < devices.size(); ++j) {
         
         // Print some introductory text
         std::cout << std::endl << "--- Investigating device " << j << " ---" << std::endl << std::endl;
         const auto & device = devices[j];

         // Detect device type
         const auto dev_type = device.type();
         std::cout << "Device type is ";
         if(dev_type & CL_DEVICE_TYPE_CUSTOM) {
            std::cout << "CUSTOM";
         } else if(dev_type & (CL_DEVICE_TYPE_DEFAULT | CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR)) {
            if(dev_type & CL_DEVICE_TYPE_DEFAULT) {
               std::cout << "DEFAULT ";
            }
            if(dev_type & CL_DEVICE_TYPE_CPU) {
               std::cout << "CPU ";
            }
            if(dev_type & CL_DEVICE_TYPE_GPU) {
               std::cout << "GPU ";
            }
            if(dev_type & CL_DEVICE_TYPE_ACCELERATOR) {
               std::cout << "ACCELERATOR ";
            }
         } else {
            std::cout << "UNKNOWN";
         }
         std::cout << std::endl;

         // Detect vendor ID
         std::cout << "Device vendor ID is " << device.vendor_id() << std::endl;

         // Detect number of compute units
         std::cout << "Device has " << device.max_compute_units() << " compute units" << std::endl;

         // Determine the maximal number of work-items in each dimension of a work-group
         const auto max_work_item_sizes = device.max_work_item_sizes();
         std::cout << "Maximal per-dimension amounts of work items are (";
         for(size_t dim = 0; dim < max_work_item_sizes.size(); ++dim) {
            std::cout << max_work_item_sizes[dim];
            if(dim != max_work_item_sizes.size() - 1) std::cout << ", ";
         }
         std::cout << ")" << std::endl;

         // Determine the maximal amount of work-items in a work-group
         std::cout << "Maximal amount of work items in a work group is " << device.max_work_group_size() << std::endl;

         // Determine the preferred vector widths
         std::cout << "Preferred vector widths are..." << std::endl;
         std::cout << " * " << device.preferred_vector_width_char() << " for char" << std::endl;
         std::cout << " * " << device.preferred_vector_width_short() << " for short" << std::endl;
         std::cout << " * " << device.preferred_vector_width_int() << " for int" << std::endl;
         std::cout << " * " << device.preferred_vector_width_long() << " for long" << std::endl;
         std::cout << " * " << device.preferred_vector_width_float() << " for float" << std::endl;
         std::cout << " * " << device.preferred_vector_width_double() << " for double" << std::endl;
         std::cout << " * " << device.preferred_vector_width_half() << " for half" << std::endl;

         // Determine the native ISA vector widths
         std::cout << "Native ISA vector widths are..." << std::endl;
         std::cout << " * " << device.native_vector_width_char() << " for char" << std::endl;
         std::cout << " * " << device.native_vector_width_short() << " for short" << std::endl;
         std::cout << " * " << device.native_vector_width_int() << " for int" << std::endl;
         std::cout << " * " << device.native_vector_width_long() << " for long" << std::endl;
         std::cout << " * " << device.native_vector_width_float() << " for float" << std::endl;
         std::cout << " * " << device.native_vector_width_double() << " for double" << std::endl;
         std::cout << " * " << device.native_vector_width_half() << " for half" << std::endl;

         // Determine the maximal clock speed
         std::cout << "Maximum configured clock frequency is " << device.max_clock_frequency() << " MHz" << std::endl;

         // Determine default device addressing width
         std::cout << "Device defaults to " << device.address_bits() << "-bit addressing" << std::endl;

         // Determine the maximal size of object allocation (in bytes)
         const auto max_alloc_size_in_mb = device.max_mem_alloc_size() / (1024 * 1024);
         std::cout << "Maximum memory allocation size is about " << max_alloc_size_in_mb << " MB" << std::endl;

         // Investigate device image support
         if(device.image_support()) {
            std::cout << "Device has support for images" << std::endl;
            std::cout << "Kernels may read from " << device.max_read_image_args() << " images and write to " << device.max_write_image_args() << " images" << std::endl;
            std::cout << "2D images may reach maximal dimensions (" << device.image2d_max_width() << ", " << device.image2d_max_height() << ")" << std::endl;
            std::cout << "3D images may reach maximal dimensions (" << device.image3d_max_width() << ", " << device.image3d_max_height() << ", " << device.image3d_max_depth() << ")" << std::endl;
            std::cout << "1D images created from a buffer may have at most " << device.image_max_buffer_size() << " pixels" << std::endl;
            std::cout << "1D and 2D image arrays may have at most " << device.image_max_array_size() << " layers" << std::endl;
            std::cout << "Kernels may use at most " << device.max_samplers() << " samplers" << std::endl;
         } else {
            std::cout << "Device does not have support for images" << std::endl;
         }

         // Determine the kernel argument size limit
         std::cout << "Kernel arguments are limited to " << device.max_parameter_size() << " bytes" << std::endl;

         // Determine the device's memory alignment requirements
         std::cout << "Device memory objects must be aligned on a " << device.mem_base_addr_align() << "-bit boundary" << std::endl;

         // Investigate device floating-point configuration
         const auto print_support_status = [](bool status) {
            if(!status) std::cout << "not ";
            std::cout << "supported" << std::endl;
         };
         const auto analyze_fp_support = [&print_support_status](cl_device_fp_config configuration, bool display_divide_sqrt_rounding) {
            if(configuration != 0) {
               std::cout << "supported, with the following specifics :" << std::endl;
               std::cout << " * Denorms are "; print_support_status(configuration & CL_FP_DENORM);
               std::cout << " * INF and quiet NaNs are "; print_support_status(configuration & CL_FP_INF_NAN);
               std::cout << " * Round to nearest even rounding mode is "; print_support_status(configuration & CL_FP_ROUND_TO_NEAREST);
               std::cout << " * Round to zero rounding mode is "; print_support_status(configuration & CL_FP_ROUND_TO_ZERO);
               std::cout << " * Round to +/-infinity rounding mode is "; print_support_status(configuration & CL_FP_ROUND_TO_INF);
               std::cout << " * IEEE754-2008 fused multiply-add is "; print_support_status(configuration & CL_FP_FMA);
               if(display_divide_sqrt_rounding) {
                  std::cout << " * Divide and sqrt are ";
                  if((configuration & CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT) == 0) std::cout << "not ";
                  std::cout << "correctly rounded (as in IEEE754 spec)" << std::endl;
               }
               if(configuration & CL_FP_SOFT_FLOAT) std::cout << " * Basic floating-point operations are implemented in software" << std::endl;
            } else {
               std::cout << "unsupported" << std::endl;
            }
         };
         std::cout << "Single-precision floats are "; analyze_fp_support(device.single_fp_config(), true);
         std::cout << "Double-precision floats are "; analyze_fp_support(device.double_fp_config(), false);

         // Investigate global memory caching
         const auto cache_type = device.global_mem_cache_type();
         if(cache_type != CL_NONE) {
            std::cout << "Global memory caching is supported for ";
            switch(cache_type) {
               case CL_READ_ONLY_CACHE:
                  std::cout << "reads";
                  break;
               case CL_READ_WRITE_CACHE:
                  std::cout << "reads and writes";
                  break;
               default:
                  std::cout << "unknown operations";
            }
            std::cout << std::endl;
            std::cout << "Global memory cache lines are " << device.global_mem_cacheline_size() << " bytes long" << std::endl;
            std::cout << "Global memory cache, overall, is " << device.global_mem_cache_size() << " bytes" << std::endl;
         } else {
            std::cout << "Global memory caching is unsupported" << std::endl;
         }

         // Determine the size of global memory
         const auto global_mem_size_in_mb = device.global_mem_size() / (1024 * 1024);
         std::cout << "Global memory is about " << global_mem_size_in_mb << " MB large" << std::endl;

         // Determine constant buffer properties
         const auto max_constbuf_size_in_kb = device.max_constant_buffer_size() / 1024;
         std::cout << "Constant buffers should be no larger than about " << max_constbuf_size_in_kb << " KB" << std::endl;
         std::cout << "Kernels should have no more than " << device.max_constant_args() << " constant arguments" << std::endl;

         // Investigate local memory properties
         const auto local_mem_type = device.local_mem_type();
         if(local_mem_type != CL_NONE) {
            std::cout << "Device has local memory support";
            switch(local_mem_type) {
               case CL_LOCAL:
                  std::cout << ", with dedicated storage";
                  break;
               case CL_GLOBAL:
                  std::cout << " but will spill it to global memory";
            }
            std::cout << std::endl;
            const auto local_mem_size_in_kb = device.local_mem_size() / 1024;
            std::cout << "Local memory is about " << local_mem_size_in_kb << " KB large" << std::endl;
         } else {
            std::cout << "Device does not support local memory" << std::endl;
         }

         // Investigate error correction code status
         std::cout << "Device ";
         if(device.error_correction_support()) {
            std::cout << "implements ";
         } else {
            std::cout << "does not implement ";
         }
         std::cout << "ECC for compute memory accesses" << std::endl;

         // Investigate unified memory support
         std::cout << "Device and host ";
         if(device.unified_memory()) {
            std::cout << "share ";
         } else {
            std::cout << "do not share ";
         }
         std::cout << "a unified memory subsystem" << std::endl;

         // Determine profiling timer resolution
         std::cout << "Device profiling timer has a resolution of " << device.profiling_timer_resolution() << " ns" << std::endl;

         // Determine endianness
         std::cout << "Device is ";
         if(device.endian_little()) {
            std::cout << "little";
         } else {
            std::cout << "big";
         }
         std::cout << "-endian" << std::endl;

         // Determine availability
         std::cout << "Device is ";
         if(!device.available()) {
            std::cout << "not ";
         }
         std::cout << "available" << std::endl;

         // Investigate device compiler and linker support
         const auto compiler_available = device.compiler_available();
         const auto linker_available = device.linker_available();
         std::cout << "Implementation can";
         if(compiler_available || linker_available) {
            if(compiler_available) std::cout << " compile ";
            if(compiler_available && linker_available) std::cout << "and ";
            if(linker_available) std::cout << "link ";
         } else {
            std::cout << "not compile nor link ";
         }
         std::cout << "OpenCL C code for this device" << std::endl;
         
         // Investigate device execution capabilities
         const auto execution_capabilities = device.execution_capabilities();
         std::cout << "Device can";
         if((execution_capabilities & CL_EXEC_KERNEL) == 0) std::cout << "not";
         std::cout << " execute OpenCL kernels" << std::endl;
         std::cout << "Device can";
         if((execution_capabilities & CL_EXEC_NATIVE_KERNEL) == 0) std::cout << "not";
         std::cout << " execute native kernels" << std::endl;

         // Investigate device command queue properties
         const auto supported_queue_properties = device.queue_properties();
         std::cout << "Device can";
         if((supported_queue_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) == 0) std::cout << "not";
         std::cout << " execute commands out of order" << std::endl;
         std::cout << "Device can";
         if((supported_queue_properties & CL_QUEUE_PROFILING_ENABLE) == 0) std::cout << "not";
         std::cout << " profile commands in its command queues" << std::endl;

         // Investigate built-in kernels
         const auto built_in_kernels = device.built_in_kernels();
         if(built_in_kernels.size() > 0) {
            std::cout << "The following built-in kernels are supported :" << std::endl;
            for(const auto & kernel : built_in_kernels) {
               std::cout << " * " << kernel << std::endl;
            }
         } else {
            std::cout << "There are no supported built-in kernels" << std::endl;
         }

         // Investigate platform information
         CLplusplus::Platform device_platform(device.raw_platform_id());
         std::cout << "Device claims to be rattached to platform " << device_platform.name() << std::endl;

         // Detect device name and vendor
         std::cout << "Device name is " << device.name() << std::endl;
         std::cout << "Device vendor is " << device.vendor() << std::endl;

         // Detect device driver version
         const auto dev_driver_version = device.driver_version();
         std::cout << "Device driver version is " << dev_driver_version.major << "." << dev_driver_version.minor << std::endl;
         
         // Detect device profile
         const auto dev_profile = device.profile();
         switch(dev_profile) {
            case CLplusplus::Profile::Full:
               std::cout << "Device implements OpenCL Full Profile" << std::endl;
               break;
            case CLplusplus::Profile::Embedded:
               std::cout << "Device implements OpenCL Embedded Profile" << std::endl;
               break;
            default:
               std::cout << "Device implements an unknown OpenCL profile" << std::endl;
         }

         // Detect device OpenCL version
         const auto dev_version = device.version();
         std::cout << "Device OpenCL version is " << dev_version.major << "." << dev_version.minor << " [" << dev_version.vendor_specific_info << "]" << std::endl;

         // Detect maximal supported OpenCL C version
         const auto dev_opencl_c_version = device.opencl_c_version();
         std::cout << "Maximal supported OpenCL C version is " << dev_opencl_c_version.major << "." << dev_opencl_c_version.minor << " [" << dev_opencl_c_version.vendor_specific_info << "]" << std::endl;

         // Detect extensions
         const auto dev_extensions = device.extensions();
         std::cout << "Device supports " << dev_extensions.size() << " extensions: ";
         for(const auto & extension : dev_extensions) {
            std::cout << extension << " ";
         }
         std::cout << std::endl;

         // Detect printf buffer size
         const auto printf_bufsize_in_kb = device.printf_buffer_size() / 1024;
         std::cout << "Kernel printf buffer can store about " << printf_bufsize_in_kb << " KB of output" << std::endl;

         // Investigate preferred interop synchronization
         std::cout << "In interop scenarii, ";
         if(device.preferred_interop_user_sync()) {
            std::cout << "the user should manage memory object synchronization";
         } else {
            std::cout << "the device can synchronize shared memory objects";
         }
         std::cout << std::endl;

         // Investigate whether our device is a sub-device or a root device. In the former case, probe sub-device-specific properties.
         if(device.has_parent_device()) {
            std::cout << "Parent device is called " << device.parent_device().name() << std::endl;
            const auto partition_type = device.partition_type();
            std::cout << "Sub-device was created by ";
            const auto first_partition_property = *(partition_type.begin()); // NOTE : This assumes that the partition only has one property, which is the case in OpenCL 1.2 but may change in the future
            switch(first_partition_property.name()) {
               case CL_DEVICE_PARTITION_EQUALLY:
                  std::cout << "equal-size partitioning into chunks of " << first_partition_property.value() << " compute units";
                  break;
               case CL_DEVICE_PARTITION_BY_COUNTS:
                  std::cout << "user-specified partitioning with these chunk sizes : ";
                  for(const auto & chunk_size : first_partition_property) {
                     if(chunk_size != 0) std::cout << chunk_size << " ";
                  }
                  break;
               case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
                  std::cout << "automatic partitioning by affinity domain ";
                  switch(first_partition_property.value()) {
                     case CL_DEVICE_AFFINITY_DOMAIN_NUMA:
                        std::cout << "NUMA";
                        break;
                     case CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE:
                        std::cout << "L4";
                        break;
                     case CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE:
                        std::cout << "L3";
                        break;
                     case CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE:
                        std::cout << "L2";
                        break;
                     case CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE:
                        std::cout << "L1";
                        break;
                     case CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE:
                        std::cout << "NEXT";
                        break;
                     default:
                        std::cout << "<unknown>";
                  }
               default:
                  std::cout << "an unknown partitioning scheme";
            }
            std::cout << std::endl;
         } else {
            std::cout << "Device is root-level, has no parent" << std::endl;
         }

         // Check device partitioning properties
         if(device.supports_partitioning()) {
            std::cout << "Device may be partitioned in at most " << device.partition_max_sub_devices() << " sub-devices" << std::endl;
            const auto supported_partition_types = device.partition_properties();
            std::cout << "The following partition types are supported :" << std::endl;
            for(const auto & partition_type : supported_partition_types) {
               switch(partition_type) {
                  case CL_DEVICE_PARTITION_EQUALLY:
                     std::cout << " * Equal-size partitions";
                     break;
                  case CL_DEVICE_PARTITION_BY_COUNTS:
                     std::cout << " * Partitions of user-specified sizes";
                     break;
                  case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
                     std::cout << " * Automatic partitioning by affinity domain among ";
                     {
                        const auto supported_affinity_domains = device.partition_affinity_domain();
                        if(supported_affinity_domains & CL_DEVICE_AFFINITY_DOMAIN_NUMA) std::cout << "NUMA ";
                        if(supported_affinity_domains & CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE) std::cout << "L4 ";
                        if(supported_affinity_domains & CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE) std::cout << "L3 ";
                        if(supported_affinity_domains & CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE) std::cout << "L2 ";
                        if(supported_affinity_domains & CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE) std::cout << "L1 ";
                        if(supported_affinity_domains & CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE) std::cout << "NEXT ";
                     }
                     break;
                  default:
                     std::cout << " * <something unknown>";
               }
               std::cout << std::endl;
            }
         } else {
            std::cout << "Device does not support partitioning" << std::endl;
         }

      }
   }

   return 0;
}
