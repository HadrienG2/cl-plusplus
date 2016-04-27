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

#include <CL/cl.h>

#include "property_list.hpp"

namespace CLplusplus {

   namespace UnitTests {

      void run_tests_property_list() {
         #ifdef CL_VERSION_1_2
         // For this test, we will use lists of device partition properties
         using TestedProperty = cl_device_partition_property;
         using TestedPropertyList = PropertyList<TestedProperty>;

         // Check that default constructor produces an empty property list, and that accessing nonexistent elements fails
         {
            TestedPropertyList default_list{};
            check(default_list.begin() == default_list.end(), "A default-constructed property list should be empty");
   
            try {
               default_list[CL_DEVICE_PARTITION_EQUALLY];
               fail("Trying to access an element within the default list should fail");
            } catch(const InvalidArgument) {
               pass();
            } catch(...) {
               fail("Trying to access an element within the default list should throw InvalidArgument");
            }

            const TestedProperty * const opencl_version = default_list.opencl_view();
            check(*opencl_version == 0, "The OpenCL version of a the default list should also be empty");
         }

         // Check that constructing from an empty OpenCL property list also produces an empty property list
         {
            const TestedProperty empty_opencl_list[] = { 0 };
            TestedPropertyList empty_list{empty_opencl_list};

            check(empty_list.begin() == empty_list.end(), "Constructiong from an empty OpenCL list should also lead an empty property list");

            const TestedProperty * const opencl_version = empty_list.opencl_view();
            check(*opencl_version == 0, "Converting back the empty property list to its OpenCL representation should be lossless");
         }

         // Check that adding a scalar property to the list works
         {
            TestedPropertyList one_scalar_list{};
            one_scalar_list.append(CL_DEVICE_PARTITION_EQUALLY, 42);
            check(one_scalar_list.begin() != one_scalar_list.end(), "After adding a scalar element, a non-empty property list should result");

            const auto scalar_view = one_scalar_list[CL_DEVICE_PARTITION_EQUALLY];
            check(scalar_view.name() == CL_DEVICE_PARTITION_EQUALLY, "Scalar property name should be correct");
            check(scalar_view.kind() == TestedPropertyList::PropertyTag::Scalar, "The property should be correctly identified as scalar");
            check(scalar_view.value() == 42, "Accessing a scalar view should work well");
            check(*scalar_view == 42, "The syntaxic sugar for scalar view access should work too");

            auto property_iterator = one_scalar_list.begin();
            check((*property_iterator).name() == CL_DEVICE_PARTITION_EQUALLY, "begin() should provide a correct iterator on one-scalar list");
            ++property_iterator;
            check(property_iterator == one_scalar_list.end(), "There should only be one item in the iterable one-scalar list");

            const TestedProperty * const opencl_version = one_scalar_list.opencl_view();
            check(opencl_version[0] == CL_DEVICE_PARTITION_EQUALLY, "OpenCL output from one-scalar list should feature the right property type");
            check(opencl_version[1] == 42, "OpenCL output from one-scalar list should feature the right property value");
            check(opencl_version[2] == 0, "OpenCL output from one-scalar list should feature a trailing zero");
         }

         // Check that translating a one-scalar OpenCL property back and forth works
         {
            const TestedProperty opencl_one_scalar_list[] { CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN, 69, 0 };
            TestedPropertyList one_scalar_list { opencl_one_scalar_list };
            const TestedProperty * const opencl_version = one_scalar_list.opencl_view();
            check(opencl_version[0] == CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN, "Doubly translated one-scalar list should feature the right property type");
            check(opencl_version[1] == 69, "Doubly translated one-scalar list should feature the right property value");
            check(opencl_version[2] == 0, "Doubly translated one-scalar list should feature a trailing zero");
         }

         // Add a vector property to the list (std::vector version)
         {
            TestedPropertyList one_vector_list{};
            one_vector_list.append(CL_DEVICE_PARTITION_BY_COUNTS, std::vector<TestedProperty>{8, 16, 32});
            check(one_vector_list.begin() != one_vector_list.end(), "After adding a vector element, a non-empty property list should result");

            const auto vector_view = one_vector_list[CL_DEVICE_PARTITION_BY_COUNTS];
            check(vector_view.name() == CL_DEVICE_PARTITION_BY_COUNTS, "Vector property name should be correct");
            check(vector_view.kind() == TestedPropertyList::PropertyTag::Vector, "The property should be correctly identified as vector");
            check(vector_view.size() == 3, "Vector views should advertise the right size");
            check(vector_view[0] == 8, "Access to the beginning of a vector view should work");
            check(vector_view[2] == 32, "Access to the end of a vector view should work");
            check(*(vector_view.begin()) == 8, "Iterator-based access to the beginning of a vector view should work");
            check(vector_view.end() - vector_view.begin() == 3, "The iterator view of the end of a vector should be accurate");

            auto property_iterator = one_vector_list.begin();
            check((*property_iterator).name() == CL_DEVICE_PARTITION_BY_COUNTS, "begin() should provide a correct iterator on one-vector list");
            ++property_iterator;
            check(property_iterator == one_vector_list.end(), "There should only be three items in the iterable one-vector list");

            const TestedProperty * const opencl_version = one_vector_list.opencl_view();
            check(opencl_version[0] == CL_DEVICE_PARTITION_BY_COUNTS, "OpenCL output from one-vector list should feature the right property type");
            check(opencl_version[1] == 8, "OpenCL output from one-vector list should feature the right property value");
            check(opencl_version[2] == 16, "OpenCL output from one-vector list should feature the right property value");
            check(opencl_version[3] == 32, "OpenCL output from one-vector list should feature the right property value");
            check(opencl_version[4] == 0, "OpenCL output from one-vector list should feature zero-terminate the vector value");
            check(opencl_version[5] == 0, "OpenCL output from one-vector list should feature a trailing zero");
         }

         // Add a vector property to the list (Native OpenCL version)
         {
            TestedPropertyList one_vector_list{};
            const TestedProperty opencl_vector[] {64, 128, 255, 0};
            const auto vector_size = one_vector_list.append(CL_DEVICE_PARTITION_BY_COUNTS, opencl_vector);
            check(vector_size == 3, "Vector append code should correctly detect OpenCL vector size");
            check(one_vector_list.begin() != one_vector_list.end(), "After adding an OpenCL vector element, a non-empty property list should result");

            const auto vector_view = one_vector_list[CL_DEVICE_PARTITION_BY_COUNTS];
            check(vector_view.name() == CL_DEVICE_PARTITION_BY_COUNTS, "OpenCL vector property name should be correct");
            check(vector_view.kind() == TestedPropertyList::PropertyTag::Vector, "The property should be correctly identified as an OpenCL vector");
            check(vector_view.size() == 3, "OpenCL vector views should advertise the right size");
            
            const TestedProperty * const opencl_version = one_vector_list.opencl_view();
            check(opencl_version[0] == CL_DEVICE_PARTITION_BY_COUNTS, "OpenCL output from OpenCL one-vector list should feature the right property type");
            check(opencl_version[1] == 64, "OpenCL output from OpenCL one-vector list should feature the right property value");
            check(opencl_version[2] == 128, "OpenCL output from OpenCL one-vector list should feature the right property value");
            check(opencl_version[3] == 255, "OpenCL output from OpenCL one-vector list should feature the right property value");
            check(opencl_version[4] == 0, "OpenCL output from OpenCL one-vector list should feature zero-terminate the vector value");
            check(opencl_version[5] == 0, "OpenCL output from OpenCL one-vector list should feature a trailing zero");
         }

         // Check that translating a native OpenCL vector back and forth works
         {
            const TestedProperty opencl_one_vector_list[] { CL_DEVICE_PARTITION_BY_COUNTS, 64, 128, 255, 0, 0 };
            TestedPropertyList one_vector_list { opencl_one_vector_list };
            const TestedProperty * const opencl_version = one_vector_list.opencl_view();
            check(opencl_version[0] == CL_DEVICE_PARTITION_BY_COUNTS, "Doubly translated one-vector list should feature the right property type");
            check(opencl_version[1] == 64, "Doubly translated one-vector list should feature the right property value");
            check(opencl_version[2] == 128, "Doubly translated one-vector list should feature the right property value");
            check(opencl_version[3] == 255, "Doubly translated one-vector list should feature the right property value");
            check(opencl_version[4] == 0, "Doubly translated one-vector list should feature zero-terminate the vector value");
            check(opencl_version[5] == 0, "Doubly translated one-vector list should feature a trailing zero");
         }
         
         // Finish with a scalar-vector-scalar example
         {
            const TestedProperty opencl_full_list[] { CL_DEVICE_PARTITION_EQUALLY, 42,
                                                      CL_DEVICE_PARTITION_BY_COUNTS, 64, 128, 255, 0,
                                                      CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN, 69,
                                                      0 };
            TestedPropertyList full_list { opencl_full_list };

            const auto property_view_1 = full_list[CL_DEVICE_PARTITION_EQUALLY];
            check(property_view_1.kind() == TestedPropertyList::PropertyTag::Scalar, "Simple property review of the full list should lead reasonable results");
            const auto property_view_2 = full_list[CL_DEVICE_PARTITION_BY_COUNTS];
            check(property_view_2.kind() == TestedPropertyList::PropertyTag::Vector, "Simple property review of the full list should lead reasonable results");
            check(property_view_2.size() == 3, "Simple property review of the full list should lead reasonable results");
            const auto property_view_3 = full_list[CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN];
            check(property_view_3.kind() == TestedPropertyList::PropertyTag::Scalar, "Simple property review of the full list should lead reasonable results");

            auto property_iterator = full_list.begin();
            for(unsigned int i = 0; i < 3; ++i) ++property_iterator;
            check(property_iterator == full_list.end(), "There should be three items in the iterable full list");

            const TestedProperty * const opencl_version = full_list.opencl_view();
            check(opencl_version[0] == CL_DEVICE_PARTITION_EQUALLY, "OpenCL output of scalar-vector-scalar test vector should match");
            check(opencl_version[1] == 42, "OpenCL output of scalar-vector-scalar test vector should match");
            check(opencl_version[2] == CL_DEVICE_PARTITION_BY_COUNTS, "OpenCL output of scalar-vector-scalar test vector should match");
            check(opencl_version[3] == 64, "OpenCL output of scalar-vector-scalar test vector should match");
            check(opencl_version[4] == 128, "OpenCL output of scalar-vector-scalar test vector should match");
            check(opencl_version[5] == 255, "OpenCL output of scalar-vector-scalar test vector should match");
            check(opencl_version[6] == 0, "OpenCL output of scalar-vector-scalar test vector should match");
            check(opencl_version[7] == CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN, "OpenCL output of scalar-vector-scalar test vector should match");
            check(opencl_version[8] == 69, "OpenCL output of scalar-vector-scalar test vector should match");
            check(opencl_version[9] == 0, "OpenCL output of scalar-vector-scalar test vector should match");
         }
         #endif
      }

   }

}
