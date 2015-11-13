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

#ifndef INCLUDE_CL_PLUSPLUS_VERSION
#define INCLUDE_CL_PLUSPLUS_VERSION

#include <string>

#include "common.hpp"

// This code unit provides facilities for decoding OpenCL version strings
namespace CLplusplus {

   // This struct can represent any versioning information featured within OpenCL.
   struct Version {
      unsigned int major;
      unsigned int minor;
      std::string vendor_specific_info;
      bool operator==(const Version & reference);

      // This function can be used for version compatibility check, but please note that it ignores vendor-specific information
      bool operator>=(const Version & reference);
   };

   // These specific instances represent various supported OpenCL versions
   const Version version_1p2 { 1, 2, "" };

   // This function decodes an OpenCL version string, using generic format "<preamble><major>.<minor>[ <vendor_specific>]", into the struct above.
   Version decode_version_string(const std::string & version_string, const std::string & preamble, const bool has_vendor_info);

   // These are convenience wrappers to the function above for common kinds of OpenCL version strings
   Version decode_driver_version_string(const std::string & version_string);     // "<major>.<minor>"
   Version decode_opencl_version_string(const std::string & version_string);     // "OpenCL <major>.<minor> <vendor-specific>"
   Version decode_opencl_c_version_string(const std::string & version_string);   // "OpenCL C <major>.<minor> <vendor-specific>"

   namespace UnitTests {

      // Test the components of this unit
      void run_tests_version();

   }

}

#endif
