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

#include "common.hpp"
#include "queries.hpp"

namespace CLplusplus {

   std::vector<Platform> get_platforms() {
      // Check how many platforms are available on the system
      cl_uint number_of_platforms;
      throw_if_failed(clGetPlatformIDs(0, nullptr, &number_of_platforms));
      
      // Fetch the platform ID list
      cl_platform_id platforms[number_of_platforms];
      throw_if_failed(clGetPlatformIDs(number_of_platforms, platforms, nullptr));

      // Construct a vector of platform objects from it
      std::vector<Platform> result;
      result.reserve(number_of_platforms);
      for(cl_uint current_platform = 0; current_platform < number_of_platforms; ++current_platform) {
         result.emplace_back(platforms[current_platform]);
      }

      // Return the result
      return result;
   }

   std::vector<Platform> get_filtered_platforms(const PlatformPredicate & filter) {
      // Check out the full platform list into a temporary object
      const std::vector<Platform> full_platform_list = get_platforms();

      // Construct the result by selectively filtering the platform list
      std::vector<Platform> result;
      std::copy_if(full_platform_list.begin(), full_platform_list.end(), std::back_inserter(result), filter);

      // Return the filtered platform list
      return result;
   }

   std::vector<FilteredPlatform> get_filtered_devices(const PlatformPredicate & platform_filter, const DevicePredicate & device_filter) {
      // Check out the filtered platform list
      const std::vector<Platform> filtered_platform_list = get_filtered_platforms(platform_filter);

      // Prepare result storage
      std::vector<FilteredPlatform> result;

      // For each filtered platform, check out the filtered devices
      for(const auto & platform : filtered_platform_list) {
         const std::vector<Device> filtered_devices = platform.filtered_devices(device_filter);

         // If any device matches the filter, store the <platform, device list> pair in our result vector
         if(filtered_devices.size() > 0) result.emplace_back(FilteredPlatform{ platform, filtered_devices });
      }

      // At this point, we have a list of all platforms with matching devices, ready to be returned
      return result;
   }

}
