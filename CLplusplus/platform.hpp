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

#ifndef INCLUDE_CL_PLUSPLUS_PLATFORM
#define INCLUDE_CL_PLUSPLUS_PLATFORM

#include <functional>
#include <string>
#include <vector>

#include <CL/cl.h>

#include "common.hpp"
#include "device.hpp"
#include "extensions.hpp"
#include "profile.hpp"
#include "version.hpp"

// This code unit provides facilities for handling OpenCL platforms
namespace CLplusplus {

   // This class represents an OpenCL platform that can be queried in a high-level way.
   class Platform {
      public:
         // We need a valid OpenCL platform ID in order to initialize this class
         Platform(const cl_platform_id identifier);

         // Platform properties which are supported by the wrapper are directly accessible in a convenient, high-level fashion
         CLplusplus::Profile profile() const { return decode_profile_string(raw_profile_string()); }
         CLplusplus::Version version() const { return decode_opencl_version_string(raw_string_query(CL_PLATFORM_VERSION)); }
         std::string name() const { return raw_string_query(CL_PLATFORM_NAME); };
         std::string vendor() const { return raw_string_query(CL_PLATFORM_VENDOR); };
         CLplusplus::ExtensionList extensions() const { return ExtensionList{raw_string_query(CL_PLATFORM_EXTENSIONS)}; }

         // Available devices may be queried as well, using a high-level equivalent of clGetDeviceIDs, possibly with some additional user-defined device filtering
         // (to request e.g. double precision capability, minimal work group sizes, minimal storage, etc...)
         std::vector<CLplusplus::Device> devices(const cl_device_type dev_type) const;
         std::vector<CLplusplus::Device> filtered_devices(const DevicePredicate & filter, const cl_device_type dev_type = CL_DEVICE_TYPE_ALL) const;

         // Wrapper-unsupported property values may be queried in a lower-level way
         std::string raw_profile_string() const { return raw_string_query(CL_PLATFORM_PROFILE); }

         // And fully unsupported platform properties can be queried in a nearly pure OpenCL way, with some common-case usability optimizations
         std::string raw_string_query(const cl_platform_info parameter_name) const;
         size_t raw_query_output_size(const cl_platform_info parameter_name) const;
         void raw_query(const cl_platform_info parameter_name, const size_t output_storage_size, void * output_storage, size_t * actual_output_size = nullptr) const;

         // Finally, if the need arises, one can directly access the platform identifier in order to perform raw OpenCL operations
         cl_platform_id raw_identifier() const { return internal_id; }

      private:
         cl_platform_id internal_id;
   };

   // This type is used by code that needs to filter platforms according to some criteria
   using PlatformPredicate = std::function<bool(const Platform &)>;

}

#endif
