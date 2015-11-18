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

#ifndef INCLUDE_CL_PLUSPLUS_COMMON
#define INCLUDE_CL_PLUSPLUS_COMMON

#include <exception>
#include <string>
#include <vector>

#include <CL/cl.h>

// This code unit defines declarations which are common to multiple parts of the wrapper
namespace CLplusplus {

   // === STANDARD OPENCL EXCEPTIONS ===

   // Exceptions inheriting from this class are a mere translation of OpenCL return codes
   class StandardException : public std::exception {};

   // To avoid namespace clashes, these exceptions are defined in a separate namespace
   namespace StandardExceptions {
      class DeviceNotAvailable : public StandardException {};
      class DeviceNotFound : public StandardException {};
      class DevicePartitionFailed : public StandardException {};
      class ExecStatusErrorForEventsInWaitList : public StandardException {};
      class InvalidBufferSize : public StandardException {};
      class InvalidCommandQueue : public StandardException {};
      class InvalidContext : public StandardException {};
      class InvalidDevice : public StandardException {};
      class InvalidDevicePartitionCount : public StandardException {};
      class InvalidDeviceType : public StandardException {};
      class InvalidEvent : public StandardException {};
      class InvalidEventWaitList : public StandardException {};
      class InvalidHostPtr : public StandardException {};
      class InvalidMemObject : public StandardException {};
      class InvalidOperation : public StandardException {};
      class InvalidPlatform : public StandardException {};
      class InvalidProperty : public StandardException {};
      class InvalidQueueProperties : public StandardException {};
      class InvalidValue : public StandardException {};
      class MapFailure : public StandardException {};
      class MemCopyOverlap : public StandardException {};
      class MemObjectAllocationFailure : public StandardException {};
      class MisalignedSubBufferOffset : public StandardException {};
      class OutOfHostMemory : public StandardException {};
      class OutOfResources : public StandardException {};
   }

   // This function raises the standard exception associated to an error code, or UnknownStandardException if it is not aware of the error that occured.
   [[noreturn]] void throw_standard_exception(cl_int error_code);
   class UnknownStandardException : StandardException {};

   // This function checks the return code of an OpenCL function, and calls throw_standard_exception if the function call failed
   void throw_if_failed(cl_int return_code);

   // === WRAPPER-SPECIFIC EXCEPTIONS ===

   // Exceptions inheriting from this class are specific to this OpenCL wrapper
   class WrapperException : public std::exception {};

   // This exception will be thrown if a function is passed an invalid argument
   class InvalidArgument : public WrapperException {};

   // === OPENCL LIST PARSING ===

   // OpenCL loves character-separated lists, but these are somewhat impractical to process. So here's a function that does it.
   std::vector<std::string> decode_opencl_list(const std::string & character_separated_list, const char separator);

   // === UNIT TESTING ===

   namespace UnitTests {

      // Check if a component validates a certain property, abort with an error message otherwise
      void check(const bool assertion, const std::string & failure_message);

      // These functions define what happens when tests passes or fail
      void pass();
      [[noreturn]] void fail(const std::string & failure_message, bool remove_leading_newline = false);
      class TestFailed : public WrapperException {};

      // Test the components of this unit
      void run_tests_common();

      // Test all the components of CLplusplus
      void run_all_tests();

   }

}

#endif
