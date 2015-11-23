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
#include <iterator>

#include <CL/cl.h>

#include "platform.hpp"

namespace CLplusplus {

   Platform::Platform(const cl_platform_id identifier) :
      internal_id{identifier}
   {
      // Handle invalid platform IDs
      if(internal_id == NULL) throw InvalidArgument();
   }

   std::vector<CLplusplus::Device> Platform::devices(const cl_device_type dev_type) const {
      // Check how many devices are available on the platform
      cl_uint number_of_devices;
      throw_if_failed(clGetDeviceIDs(internal_id, dev_type, 0, nullptr, &number_of_devices));
      
      // Fetch the device ID list
      cl_device_id devices[number_of_devices];
      throw_if_failed(clGetDeviceIDs(internal_id, dev_type, number_of_devices, devices, nullptr));

      // Construct a vector of device objects from it
      std::vector<Device> result;
      result.reserve(number_of_devices);
      for(cl_uint current_device = 0; current_device < number_of_devices; ++current_device) {
         result.emplace_back(Device{devices[current_device], false});
      }

      // Return the result
      return result;
   }

   std::vector<CLplusplus::Device> Platform::filtered_devices(const DevicePredicate & filter, const cl_device_type dev_type) const {
      // Check out the full device list into a temporary object
      const std::vector<Device> full_device_list = devices(dev_type);

      // Construct the result by selectively filtering the device list
      std::vector<Device> result;
      std::copy_if(full_device_list.begin(), full_device_list.end(), std::back_inserter(result), filter);

      // Return the result
      return result;
   }

   std::string Platform::raw_string_query(cl_platform_info parameter_name) const {
      // Check how long the output string should be
      size_t output_string_length = raw_query_output_size(parameter_name);
      
      // Fetch the output string
      char output_string[output_string_length];
      raw_query(parameter_name, output_string_length, static_cast<void *>(output_string));

      // Return the result
      return std::string(output_string);
   }

   size_t Platform::raw_query_output_size(cl_platform_info parameter_name) const {
      size_t result;
      raw_query(parameter_name, 0, nullptr, &result);
      return result;
   }

   void Platform::raw_query(cl_platform_info parameter_name, size_t output_storage_size, void * output_storage, size_t * actual_output_size) const {
      throw_if_failed(clGetPlatformInfo(internal_id, parameter_name, output_storage_size, output_storage, actual_output_size));
   }

}
