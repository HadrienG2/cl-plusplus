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

#include "common.hpp"
#include "profile.hpp"

namespace CLplusplus {

   Profile decode_profile_string(const std::string & profile_string) {
      // Let us define all the supported profile strings
      static const std::string full_profile_string{"FULL_PROFILE"};
      static const std::string embedded_profile_string{"EMBEDDED_PROFILE"};

      // Check if the one we got matches these
      if(profile_string == full_profile_string) {
         return Profile::Full;
      } else if(profile_string == embedded_profile_string) {
         return Profile::Embedded;
      } else {
         throw UnsupportedProfileString();
      }
   }

   namespace UnitTests {

      void run_tests_profile() {
         // Try decoding all the current OpenCL profile strings
         check(decode_profile_string("FULL_PROFILE") == Profile::Full, "Decoding the full profile's profile string should work");
         check(decode_profile_string("EMBEDDED_PROFILE") == Profile::Embedded, "Decoding the embedded profile's profile string should work");

         // Check that trying to decode an unsupported profile string raises the right exception
         try {
            decode_profile_string("TOTALLY_NON_EXISTENT_PROFILE");
            fail("Decoding an unknown profile string should throw an exception");
         } catch(const UnsupportedProfileString) {
            pass();
         } catch(...) {
            fail("Decoding an unknown profile string should throw UnsupportedProfileString");
         }
      }

   }

}
