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

#include <iostream>

#include "common.hpp"
#include "extensions.hpp"
#include "profile.hpp"
#include "property_list.hpp"
#include "version.hpp"

namespace CLplusplus {
   
   [[noreturn]] void throw_standard_exception(cl_int error_code) {
      using namespace StandardExceptions;
      
      switch(error_code) {
         case CL_DEVICE_NOT_AVAILABLE:
            throw DeviceNotAvailable();
         case CL_DEVICE_NOT_FOUND:
            throw DeviceNotFound();
         case CL_DEVICE_PARTITION_FAILED:
            throw DevicePartitionFailed();
         case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            throw ExecStatusErrorForEventsInWaitList();
         case CL_INVALID_BUFFER_SIZE:
            throw InvalidBufferSize();
         case CL_INVALID_COMMAND_QUEUE:
            throw InvalidCommandQueue();
         case CL_INVALID_CONTEXT:
            throw InvalidContext();
         case CL_INVALID_DEVICE:
            throw InvalidDevice();
         case CL_INVALID_DEVICE_PARTITION_COUNT:
            throw InvalidDevicePartitionCount();
         case CL_INVALID_DEVICE_TYPE:
            throw InvalidDeviceType();
         case CL_INVALID_EVENT:
            throw InvalidEvent();
         case CL_INVALID_EVENT_WAIT_LIST:
            throw InvalidEventWaitList();
         case CL_INVALID_HOST_PTR:
            throw InvalidHostPtr();
         case CL_INVALID_MEM_OBJECT:
            throw InvalidMemObject();
         case CL_INVALID_OPERATION:
            throw InvalidOperation();
         case CL_INVALID_PLATFORM:
            throw InvalidPlatform();
         case CL_INVALID_PROPERTY:
            throw InvalidProperty();
         case CL_INVALID_QUEUE_PROPERTIES:
            throw InvalidQueueProperties();
         case CL_INVALID_VALUE:
            throw InvalidValue();
         case CL_MAP_FAILURE:
            throw MapFailure();
         case CL_MEM_COPY_OVERLAP:
            throw MemCopyOverlap();
         case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            throw MemObjectAllocationFailure();
         case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            throw MisalignedSubBufferOffset();
         case CL_OUT_OF_HOST_MEMORY:
            throw OutOfHostMemory();
         case CL_OUT_OF_RESOURCES:
            throw OutOfResources();
         default:
            throw UnknownStandardException();
      }
   }

   void throw_if_failed(cl_int return_code) {
      if(return_code != CL_SUCCESS) throw_standard_exception(return_code);
   }

   std::vector<std::string> decode_opencl_list(const std::string & character_separated_list, const char separator) {
      // Prepare our vector of results
      std::vector<std::string> result;

      // If the list is empty, return an empty vector
      if(character_separated_list.length() == 0) return result;

      // Extract the first item of the list (if any)
      auto separator_position = character_separated_list.find(separator);
      if(separator_position == std::string::npos) {
         result.emplace_back(character_separated_list);
         return result;
      } else {
         if(separator_position > 0) result.emplace_back(character_separated_list.substr(0, separator_position));
      }

      // Extract the items from the middle of the list
      auto last_separator_position = separator_position;
      separator_position = character_separated_list.find(separator, separator_position + 1);
      while(separator_position != std::string::npos) {
         // Extract a new list item if the OpenCL implementation is not trying to trick us with a double separator
         auto item_length = separator_position - last_separator_position - 1;
         if(item_length > 0) result.emplace_back(character_separated_list.substr(last_separator_position + 1, item_length));

         // Determine the position of the next separator in the list (if any)
         last_separator_position = separator_position;
         separator_position = character_separated_list.find(separator, separator_position + 1);
      }

      // Extract the last item of the list (if any, some OpenCL implementations leave a trailing separator...)
      if(last_separator_position != character_separated_list.length() - 1) {
         result.emplace_back(character_separated_list.substr(last_separator_position + 1));
      }

      // Return the result
      return result;
   }

   namespace UnitTests {

      void check(const bool assertion, const std::string & failure_message) {
         if(assertion) {
            pass();
         } else {
            fail(failure_message);
         }
      }

      void pass() {}

      [[noreturn]] void fail(const std::string & failure_message, bool remove_leading_newline) {
         if(remove_leading_newline == false) std::cout << std::endl;
         std::cout << "TEST FAILED : " << failure_message << std::endl;
         throw TestFailed();
      }

      void run_tests_common() {
         // Check if throwing an exception works
         try {
            throw_standard_exception(CL_INVALID_VALUE);
            fail("Throwing a standard OpenCL exception should work");
         } catch(const StandardExceptions::InvalidValue) {
            pass();
         } catch(...) {
            fail("Throwing a standard OpenCL exception should result in the right kind of exception being raised");
         }

         // Check that throw_if_failed will not raise exceptions thoughtlessly
         try {
            throw_if_failed(CL_SUCCESS);
            pass();
         } catch(...) {
            fail("throw_if_failed() should not raise an exception in reaction to CL_SUCCESS");
         }

         // Check that it will raise the right exception if that is warranted
         try {
            throw_if_failed(CL_INVALID_VALUE);
            fail("throw_if_failed() should raise an exception if passed an OpenCL error code");
         } catch(const StandardExceptions::InvalidValue) {
            pass();
         }

         // Try decoding empty OpenCL string lists
         using StringVector = std::vector<std::string>;
         check(decode_opencl_list("", ',') == StringVector{}, "Decoding an empty list should lead an empty result");
         check(decode_opencl_list("   ", ' ') == StringVector{}, "Decoding an list with nothing but separators should lead an empty result");
         check(decode_opencl_list(";;;", ';') == StringVector{}, "Decoding an list with nothing but separators should lead an empty result");

         // Try decoding one-, two, three- and four-element lists
         check(decode_opencl_list("this", '@') == StringVector{"this"}, "Decoding a one-element list should work");
         check(decode_opencl_list("this$is", '$') == StringVector{"this", "is"}, "Decoding a two-element list should work");
         check(decode_opencl_list("this-is-sparta", '-') == StringVector{"this", "is", "sparta"}, "Decoding a three-element list should work");
         check(decode_opencl_list("this.is.sparta.!!!", '.') == StringVector{"this", "is", "sparta", "!!!"}, "Decoding a four-element list should work");

         // Add spurious separators in various places and check that they are ignored
         check(decode_opencl_list("...this.is.sparta.!!!", '.') == StringVector{"this", "is", "sparta", "!!!"}, "Decoding a list with spurious separators in front should work");
         check(decode_opencl_list("...this...is...sparta...!!!", '.') == StringVector{"this", "is", "sparta", "!!!"}, "Decoding a list with spurious separators in the middle should work");
         check(decode_opencl_list("...this...is...sparta...!!!...", '.') == StringVector{"this", "is", "sparta", "!!!"}, "Decoding a list with spurious separators in the end should work");
      }

      void run_all_tests() {
         // NOTE : At this point, code units which directly interface with hardware are not tested, because I cannot think of a good way to do it
         try { run_tests_common(); } catch(...) { fail("Some unit tests failed for common.hpp", true); }
         try { run_tests_extensions(); } catch(...) { fail("Some unit tests failed for extensions.hpp", true); }
         try { run_tests_profile(); } catch(...) { fail("Some unit tests failed for profile.hpp", true); }
         try { run_tests_property_list(); } catch(...) { fail("Some unit tests failed for property_list.hpp", true); }
         try { run_tests_version(); } catch(...) { fail("Some unit tests failed for version.hpp", true); }
         std::cout << "All unit tests passed !" << std::endl;
      }

   }

}
