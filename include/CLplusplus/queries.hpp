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

#ifndef INCLUDE_CL_PLUSPLUS_QUERIES
#define INCLUDE_CL_PLUSPLUS_QUERIES

#include <vector>

#include "device.hpp"
#include "platform.hpp"

// This code unit provides high-level ways to query the OpenCL subsystem for e.g. platforms and devices
namespace CLplusplus {

   // This function returns platform classes for all the OpenCL platforms on the system.
   std::vector<CLplusplus::Platform> get_platforms();
   
   // This is a variant of get_platforms which only returns OpenCL platforms which match a certain user-defined predicate.
   // It should be used when targeting platforms which match a certain OpenCL version, profile, extensions...
   std::vector<CLplusplus::Platform> get_filtered_platforms(const PlatformPredicate & filter);

   // It is also possible to filter both platforms and devices, and get a pretty summary of all devices that match, sorted by platform
   struct FilteredPlatform {
      CLplusplus::Platform platform;
      std::vector<CLplusplus::Device> filtered_devices;
   };
   std::vector<FilteredPlatform> get_filtered_devices(const PlatformPredicate & platform_filter, const DevicePredicate & device_filter);

}

#endif
