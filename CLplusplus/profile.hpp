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

#ifndef INCLUDE_CL_PLUSPLUS_PROFILE
#define INCLUDE_CL_PLUSPLUS_PROFILE

#include <string>

#include "common.hpp"

// This code unit defines facilities for handling OpenCL profiles
namespace CLplusplus {

   // This enum represents the various profiles supported by OpenCL.
   enum class Profile {Full, Embedded};

   // This function converts an OpenCL profile string to the enum class above.
   // If the platform provides an unexpected profile string, the following exception will be raised.
   Profile decode_profile_string(const std::string & profile_string);
   class UnsupportedProfileString : public WrapperException {};

   namespace UnitTests {

      // Test the components of this unit
      void run_tests_profile();

   }

}

#endif
