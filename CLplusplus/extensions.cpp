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

#include <algorithm>
#include <unordered_map>

#include "common.hpp"
#include "extensions.hpp"

namespace CLplusplus {

   bool ExtensionList::contains(const std::vector<std::string> & extensions) const {
      // Build an extension name -> boolean map featuring all the extensions that we are looking for
      using ExtensionStatus = std::unordered_map<std::string, bool>;
      ExtensionStatus extension_status(extensions.size());
      for(const auto & extension : extensions) {
         extension_status.insert({extension, false});
      }

      // Search our extension list for the requested extensions, indicating in the map whether we found them
      for(const auto & extension : contents) {
         const auto status_location = extension_status.find(extension);
         if(status_location != extension_status.end()) {
            status_location->second = true;
         }
      }

      // Check if we found all the extensions we were looking for
      return std::all_of(extension_status.begin(), extension_status.end(),
                         [](const ExtensionStatus::value_type & status) -> bool { return status.second; });
   }

   namespace UnitTests {

      void run_tests_extensions() {
         // Try decoding a simple "extension" list
         ExtensionList::StringVector expected_result{"This", "is", "blasphemy"};
         ExtensionList actual_result{"This is blasphemy"};
         check(*actual_result == expected_result, "Decoding a simple extension list should work");

         // Check that we can test extension support easily
         using StringVector = std::vector<std::string>;
         check(actual_result.contains(StringVector{}), "All extension lists should contain the null extension vector");
         check(actual_result.contains(StringVector{"This"}), "Searching for the first extension should match");
         check(actual_result.contains(StringVector{"is"}), "Searching for an extension in the middle should match");
         check(actual_result.contains(StringVector{"blasphemy"}), "Searching for the last extension should match");
         check(actual_result.contains(StringVector{"This", "is", "blasphemy"}), "Searching for all extensions should match");
      }

   }

}
