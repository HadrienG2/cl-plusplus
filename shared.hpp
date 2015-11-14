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

#ifndef INCLUDE_SHARED_HPP
#define INCLUDE_SHARED_HPP

#include <exception>
#include <iostream>
#include <utility>
#include <vector>

#include "CLplusplus/context.hpp"
#include "CLplusplus/device.hpp"
#include "CLplusplus/platform.hpp"
#include "CLplusplus/queries.hpp"

// This header and namespace contains some functionality which is shared between multiple OpenCL code examples
namespace Shared {

   // This function lets a user pick, on the command line, an OpenCL device which matches some minimal criteria
   using PlatformAndDevice = std::pair<CLplusplus::Platform, CLplusplus::Device>;
   class NoSuitableDevice : std::exception{};
   PlatformAndDevice select_device(const CLplusplus::PlatformPredicate & platform_predicate, const CLplusplus::DevicePredicate & device_predicate) {
      // Detect all OpenCL platform + device combinations which match our expectations
      const auto filtered_platforms = CLplusplus::get_filtered_devices(platform_predicate, device_predicate);

      // Abort if no platform or device match our criteria
      if(filtered_platforms.size() == 0) {
         std::cout << "No suitable OpenCL platform or device detected!" << std::endl;
         throw NoSuitableDevice();
      }

      // Construct and display a numbered list of suitable OpenCL devices
      std::cout << "Please pick an OpenCL device:" << std::endl;
      size_t device_count = 0, device_number;
      std::vector<PlatformAndDevice> possible_devices;
      for(const auto & filtered_platform : filtered_platforms) {
         const auto & platform = filtered_platform.platform;
         for(const auto & device : filtered_platform.filtered_devices) {
            possible_devices.emplace_back(PlatformAndDevice{platform, device});
            std::cout << " [" << device_count << "] " << device.name() << " (vendor ID " << device.vendor_id() << ", on platform " << platform.name() << ")" << std::endl;
            ++device_count;
         }
      }
      std::cout << std::endl;

      // Have the user select which device we should use
      std::cout << "Your choice > ";
      bool try_again;
      do {
         std::cin >> device_number;
         try_again = (std::cin.fail()) || (device_number >= device_count);
         if(try_again) {
            std::cout << "That wouldn't work. Please try again > ";
            std::cin.clear();
            std::cin.ignore();
         }
      } while(try_again);
      std::cout << std::endl;

      // Return the result
      return possible_devices[device_number];
   }

   // This function builds an OpenCL context from a (platform, device) pair, as returned by the previous function, with some default parameters suitable for example code
   CLplusplus::Context build_default_context(const PlatformAndDevice & platform_and_device) {
      // Define the context's properties
      CLplusplus::ContextProperties context_properties;
      const auto platform_id = platform_and_device.first.raw_platform_id();
      context_properties.append(CL_CONTEXT_PLATFORM, (cl_context_properties)(void *)platform_id);

      // Define a basic error callback
      const auto context_callback = [](const std::string & errinfo, const void * private_info, size_t cb) -> void {
         std::cout << std::endl << "OPENCL CONTEXT ERROR: " << errinfo << " (private info at address " << (size_t)(void*)(private_info) << ", cb is " << cb << ")" << std::endl;
      };

      // Create and return the context
      return CLplusplus::Context{context_properties, platform_and_device.second, context_callback};
   }

}

#endif
