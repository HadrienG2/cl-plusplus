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

#include <algorithm>
#include <vector>

#include "common.hpp"
#include "device.hpp"

namespace CLplusplus {

   Device::Device(const cl_device_id identifier, const bool increment_reference_count) :
      internal_id{identifier}
   {
      // Handle invalid device IDs
      if(internal_id == NULL) throw InvalidArgument();

      // Unless asked not to do so, increment the device's reference count
      if(increment_reference_count) retain();
   }

   Device::Device(const Device & source) :
      internal_id{source.internal_id}
   {
      // Whenever a copy of a reference-counted device is made, its reference count should be incremented
      retain();
   }

   Device & Device::operator=(const Device & source) {
      // Reference count considerations also apply to copy assignment operator
      internal_id = source.internal_id;
      retain();
      return *this;
   }

   std::vector<size_t> Device::max_work_item_sizes() const {
      const cl_uint number_of_dimensions = max_work_item_dimensions();
      std::vector<size_t> result(number_of_dimensions);
      raw_query(CL_DEVICE_MAX_WORK_ITEM_SIZES, number_of_dimensions * sizeof(size_t), (void *)(&result[0]));
      return result;
   }

   #ifdef CL_VERSION_1_2
   std::vector<cl_device_partition_property> Device::partition_properties() const {
      // Check if the device supports partitioning at all. If not, return an empty result.
      if(!supports_partitioning()) return std::vector<cl_device_partition_property>();

      // Check how many partition types are supported by the device
      unsigned int num_partition_types = raw_query_output_size(CL_DEVICE_PARTITION_PROPERTIES) / sizeof(cl_device_partition_property);

      // Fetch the result
      std::vector<cl_device_partition_property> result(num_partition_types);
      raw_query(CL_DEVICE_PARTITION_PROPERTIES, result.size() * sizeof(cl_device_partition_property), (void *)(&result[0]));

      // Return it
      return result;
   }

   PartitionProperties Device::partition_type() const {
      // If the device is not a sub-device, return an empty result.
      if(!has_parent_device()) return PartitionProperties{};

      // Else, check how long the partition property list is going to be
      unsigned int opencl_list_length = (raw_query_output_size(CL_DEVICE_PARTITION_TYPE) / sizeof(cl_device_partition_property));
      
      // Fetch the raw result
      cl_device_partition_property opencl_list[opencl_list_length];
      raw_query(CL_DEVICE_PARTITION_TYPE, opencl_list_length * sizeof(cl_device_partition_property), (void *)opencl_list);

      // Return a higher-level abstraction of it
      return PartitionProperties{opencl_list};
   }
   #endif

   std::string Device::raw_string_query(cl_platform_info parameter_name) const {
      // Check how long the output string should be
      size_t output_string_length = raw_query_output_size(parameter_name);
      
      // Fetch the output string
      char output_string[output_string_length];
      raw_query(parameter_name, output_string_length, (void *)output_string);

      // Return the result
      return std::string(output_string);
   }

   size_t Device::raw_query_output_size(cl_device_info parameter_name) const {
      size_t result;
      raw_query(parameter_name, 0, nullptr, &result);
      return result;
   }

   void Device::raw_query(cl_device_info parameter_name, size_t output_storage_size, void * output_storage, size_t * actual_output_size) const {
      throw_if_failed(clGetDeviceInfo(internal_id, parameter_name, output_storage_size, output_storage, actual_output_size));
   }

   #ifdef CL_VERSION_1_2
   std::vector<Device> Device::create_sub_devices(PartitionProperties & properties) {
      // Convert the provided partition property list into a zero-terminated OpenCL array
      const auto opencl_properties = properties.opencl_view();

      // Check how many devices would be returned by clCreateSubDevices
      cl_uint number_of_subdevices;
      throw_if_failed(clCreateSubDevices(internal_id, opencl_properties, 0, nullptr, &number_of_subdevices));

      // Allocate temporary storage accordingly, then create the subdevices
      cl_device_id raw_result[number_of_subdevices];
      throw_if_failed(clCreateSubDevices(internal_id, opencl_properties, number_of_subdevices, raw_result, nullptr));

      // Construct device classes for all the subdevices
      std::vector<Device> result;
      result.reserve(number_of_subdevices);
      for(size_t i = 0; i < number_of_subdevices; ++i) {
         result.emplace_back(Device{raw_result[i], false});
      }
      
      // Return the result
      return result;
   }
   #endif

   void Device::retain() const {
      throw_if_failed(clRetainDevice(internal_id));
   }

   void Device::release() {
      throw_if_failed(clReleaseDevice(internal_id));
   }

}
