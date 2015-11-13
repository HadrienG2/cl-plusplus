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

#include <stdexcept>

#include "common.hpp"
#include "version.hpp"

namespace CLplusplus {

   bool Version::operator==(const Version & reference) {
      return (major == reference.major) && (minor == reference.minor) && (vendor_specific_info == reference.vendor_specific_info);
   }

   bool Version::operator>=(const Version & reference) {
      return (major > reference.major) || ((major == reference.major) && (minor >= reference.minor));
   }

   Version decode_version_string(const std::string & version_string, const std::string & preamble, const bool has_vendor_info) {
      // Check that the result looks like a valid version string
      const unsigned int preamble_length = preamble.length();
      const unsigned int minimum_length = preamble_length + 3 + ((has_vendor_info) ? 1 : 0);
      if((version_string.length() < minimum_length) || (version_string.substr(0, preamble_length) != preamble)) throw InvalidArgument();

      // Locate the major OpenCL version
      const auto dot_position = version_string.find('.', preamble_length + 1);
      if(dot_position == std::string::npos) throw InvalidArgument();
      const std::string major_version_string = version_string.substr(preamble_length, dot_position - preamble_length);      

      // Convert it to its numerical value
      unsigned int major_version;
      try {
         major_version = std::stoul(major_version_string);
      } catch(const std::invalid_argument &) {
         throw InvalidArgument();
      }
      
      // Locate the minor OpenCL version, using the vendor info's leading space to guide us if it is available
      size_t space_position = 0;
      std::string minor_version_string;
      if(has_vendor_info) {
         space_position = version_string.find(' ', dot_position + 1);
         if(space_position == std::string::npos) throw InvalidArgument();
         minor_version_string = version_string.substr(dot_position + 1, space_position - dot_position - 1);
      } else {
         minor_version_string = version_string.substr(dot_position + 1);
      }

      // Convert it to its numerical value
      unsigned int minor_version;
      try {
         minor_version = std::stoul(minor_version_string);
      } catch(const std::invalid_argument &) {
         throw InvalidArgument();
      }

      // Extract the vendor info, if any
      std::string vendor_info;
      if(has_vendor_info) {
         vendor_info = version_string.substr(space_position + 1);
      }

      // Produce the platform version output
      return { major_version, minor_version, vendor_info };
   }

   Version decode_driver_version_string(const std::string & version_string) {
      return decode_version_string(version_string, "", false);
   }

   Version decode_opencl_version_string(const std::string & version_string) {
      return decode_version_string(version_string, "OpenCL ", true);
   }
   
   Version decode_opencl_c_version_string(const std::string & version_string) {
      return decode_version_string(version_string, "OpenCL C ", true);
   }

   namespace UnitTests {

      void run_tests_version() {
         // Check that version comparison works
         check(Version{ 99, 1, "absolutely weird stuff" } >= Version{ 99, 1, "random gibberish" }, "Version comparison should work when all version numbers are the same");
         check(Version{ 99, 1, "absolutely weird stuff" } >= Version{ 99, 0, "random gibberish" }, "Version comparison should work when only minor versions differ");
         check((Version{ 99, 1, "absolutely weird stuff" } >= Version{ 99, 2, "random gibberish" }) == false, "Version comparison should work when only minor versions differ");
         check(Version{ 100, 1, "absolutely weird stuff" } >= Version{ 99, 1, "random gibberish" }, "Version comparison should work when only major versions differ");
         check((Version{ 100, 1, "absolutely weird stuff" } >= Version{ 101, 1, "random gibberish" }) == false, "Version comparison should work when only major versions differ");
         check(Version{ 100, 1, "absolutely weird stuff" } >= Version{ 99, 2, "random gibberish" }, "Version comparison should work when major and minor versions differ");
         check((Version{ 100, 2, "absolutely weird stuff" } >= Version{ 101, 1, "random gibberish" }) == false, "Version comparison should work when major and minor versions differ");

         // Torture-test the main version string decoder for all possible ways to make it pass or fail. Start with the basics...
         check(decode_version_string("0.0", "", false) == Version{ 0, 0, "" }, "Basic version string decoding should work");
         try {
            decode_version_string(".0", "", false);
            fail("Decoding a version string without a major version should fail");
         } catch(const InvalidArgument) {
            pass();
         } catch(...) {
            fail("Decoding a version string without a major version should raise InvalidArgument");
         }
         try {
            decode_version_string("00", "", false);
            fail("Decoding a version string without a version separator should fail");
         } catch(const InvalidArgument) {
            pass();
         } catch(...) {
            fail("Decoding a version string without a version separator should raise InvalidArgument");
         }
         try {
            decode_version_string("0.", "", false);
            fail("Decoding a version string without a minor version should fail");
         } catch(const InvalidArgument) {
            pass();
         } catch(...) {
            fail("Decoding a version string without a minor version should raise InvalidArgument");
         }
         try {
            decode_version_string("", "", false);
            fail("Decoding an empty version string should fail");
         } catch(const InvalidArgument) {
            pass();
         } catch(...) {
            fail("Decoding an empty version string should raise InvalidArgument");
         }
         
         // ...then test preambles...
         check(decode_version_string("Preamble 0.0", "Preamble ", false) == Version{ 0, 0, "" }, "Version string decoding with a preamble should work");
         try {
            decode_version_string("Preamble0.0", "Preamble ", false);
            fail("Decoding a version string with the wrong preamble should fail");
         } catch(const InvalidArgument) {
            pass();
         } catch(...) {
            fail("Decoding an version string with the wrong preamble should raise InvalidArgument");
         }
         try {
            decode_version_string("reamble 0.0", "Preamble ", false);
            fail("Decoding a version string with the wrong preamble should fail");
         } catch(const InvalidArgument) {
            pass();
         } catch(...) {
            fail("Decoding an version string with the wrong preamble should raise InvalidArgument");
         }
         try {
            decode_version_string("0.0", "Preamble ", false);
            fail("Decoding a version string with the wrong preamble should fail");
         } catch(const InvalidArgument) {
            pass();
         } catch(...) {
            fail("Decoding an version string with the wrong preamble should raise InvalidArgument");
         }
         
         // ...test nonzero major and minor versions...
         check(decode_version_string("Preamble 123.0", "Preamble ", false) == Version{ 123, 0, "" }, "Version string decoding with a major version should work");
         check(decode_version_string("Preamble 123.234", "Preamble ", false) == Version{ 123, 234, "" }, "Version string decoding with a minor version should work");

         // ...and test vendor specific strings...
         check(decode_version_string("Preamble 123.234 ", "Preamble ", true) == Version{ 123, 234, "" }, "Version string decoding with an empty vendor-specific string should work");
         check(decode_version_string("Preamble 123.234 Some Gibberish", "Preamble ", true) == Version{ 123, 234, "Some Gibberish" }, "Version string decoding with a nontrivial vendor-specific string should work");
         try {
            decode_version_string("Preamble 123.234", "Preamble ", true);
            fail("Decoding verison strings that should have a vendor-specific epilogue but don't should raise an exception");
         } catch(const InvalidArgument) {
            pass();
         } catch(...) {
            fail("Decoding verison strings that should have a vendor-specific epilogue but don't should raise InvalidArgument");
         }

         // Quickly check that specialzed version string decoding leads the right results
         check(decode_driver_version_string("123.234") == Version{ 123, 234, "" }, "Driver version decoding should work");
         check(decode_opencl_version_string("OpenCL 12.34 Vendor-specific gibberish") == Version{ 12, 34, "Vendor-specific gibberish" }, "OpenCL version decoding should work");
         check(decode_opencl_c_version_string("OpenCL C 54.32 Vendor specific stuff") == Version{ 54, 32, "Vendor specific stuff" }, "OpenCL C version decoding should work");
      }

   }

}
