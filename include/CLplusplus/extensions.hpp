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

#ifndef INCLUDE_CL_PLUSPLUS_EXTENSIONS
#define INCLUDE_CL_PLUSPLUS_EXTENSIONS

#include <string>
#include <vector>

#include "common.hpp"

// This code unit provides facilities for dealing with OpenCL's space-separated extension lists
namespace CLplusplus {

    // This class manages an OpenCL extension list in a high level way
    class ExtensionList {
        public:
            using StringVector = std::vector<std::string>;
            explicit ExtensionList(const std::string & space_separated_extension_list) : contents{decode_opencl_list(space_separated_extension_list, ' ')} {}
            
            const StringVector & operator*() const { return contents; }
            bool contains(const std::vector<std::string> & extensions) const;
         
        private:
            StringVector contents;
    };

    namespace UnitTests {

        // Test the components of this unit
        void run_tests_extensions();

    }

}

#endif
