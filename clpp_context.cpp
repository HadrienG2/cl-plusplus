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

#include <CL/cl.h>

#include "CLplusplus/context.hpp"
#include "CLplusplus/device.hpp"
#include "CLplusplus/platform.hpp"
#include "CLplusplus/version.hpp"

#include "shared.hpp"

// This program demonstrates context creation and basic handling in CLplusplus
int main() {
   // This is the minimal OpenCL version that we will target
   const CLplusplus::Version target_version = CLplusplus::version_1p2;

   // Have the user select a suitable device, according to some criteria (see shared.hpp for more details)
   Shared::PlatformAndDevice selected_platform_and_device = Shared::select_device(
      [&](const CLplusplus::Platform & platform) -> bool {
         return (platform.version() >= target_version);        // Platform OpenCL version is recent enough
      },
      [&](const CLplusplus::Device & device) -> bool {
         if(device.version() < target_version) return false;   // OpenCL platforms may support older-generation devices, which we need to eliminate
         return device.available() &&                          // Device is available for compute purposes
                device.endian_little();                        // Device is little-endian
      }
   );

   // Define the properties of the OpenCL context which we are going to create
   CLplusplus::ContextProperties context_properties;
   const auto platform_id = selected_platform_and_device.first.raw_identifier();
   context_properties.append(CL_CONTEXT_PLATFORM, (cl_context_properties)(void *)platform_id);

   // Define a callback to be called in case of a context error
   const auto context_callback = [](const std::string & errinfo, const void * private_info, size_t cb) -> void {
      std::cout << std::endl << "OPENCL CONTEXT ERROR: " << errinfo << " (private info at address " << (size_t)(void*)(private_info) << ", cb is " << cb << ")" << std::endl;
   };

   // Create the context
   const CLplusplus::Context context{context_properties, selected_platform_and_device.second, context_callback};

   // Display context properties
   std::cout << "Generated OpenCL context features " << context.num_devices() << " device(s) :" << std::endl;

   for(const auto & device : context.devices()) {
      std::cout << " * " << device.name() << " (vendor ID " << device.vendor_id() << ")" << std::endl;
   }
   
   std::cout << "The context was created with the following properties :" << std::endl;
   for(const auto & property : context.properties()) {
      switch(property.name()) {
         case CL_CONTEXT_PLATFORM:
            {
               const CLplusplus::Platform platform((cl_platform_id)(void *)property.value());
               std::cout << " * Platform is " << platform.name() << std::endl;
            }
            break;
         case CL_CONTEXT_INTEROP_USER_SYNC:
            {
               std::cout << " * In interop scenarii, ";
               const bool user_is_responsible = (property.value() == CL_TRUE);
               if(user_is_responsible) {
                  std::cout << "the user is responsible for OpenCL-graphics synchronization";
               } else {
                  std::cout << "OpenCL-graphics synchronization is managed by the platform";
               }
               std::cout << std::endl;
            }
            break;
         default:
            std::cout << " * <Some unrecognized property>" << std::endl;
      }
   }

   return 0;
}
