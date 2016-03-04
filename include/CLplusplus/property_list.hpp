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

#ifndef INCLUDE_CL_PLUSPLUS_PROPERTY_LIST
#define INCLUDE_CL_PLUSPLUS_PROPERTY_LIST

#include <utility>
#include <vector>

#include "common.hpp"

// This code unit provides a way to manage OpenCL-style zero-terminated lists of scalar and vectorial numerical properties in a higher-level way
namespace CLplusplus {

   // This class provides a higher-level and much safer view of OpenCL property lists
   template<typename PropertyType> class PropertyList {
      public:
         // This exception will be thrown if the property list code encounters an OpenCL property which it cannot handle yet.
         class UnsupportedProperty : public CLplusplus::WrapperException {};

         // Property lists can be fully managed by the class, or created from a valid OpenCL input
         PropertyList() {}
         PropertyList(const PropertyType * const opencl_property_list);

         // It is important to mark a distinction between property names and value, even if OpenCL, being a C API, cannot do so itself
         using PropertyName = PropertyType;
         using ScalarValue = PropertyType;

         // We will also tag property values to know whether they are scalar or vectors, as we store those in different ways
         enum class PropertyTag : PropertyType { Scalar, Vector };

         // Properties can be accessed using read-only views, as follows
         class PropertyView {
            public:
               // A property view is made from a read-only pointer into the PropertyList's internal data
               PropertyView(const PropertyType * target) : property_name{target[0]}, tag{static_cast<PropertyTag>(target[1])}, value_ptr{target + 2}, vector_data_ptr{target + 3} {}

               // This interface can be used to query what kind of property we are observing
               PropertyName name() const { return property_name; }
               PropertyTag kind() const { return tag; }

               // This interface can be used to probe scalar properties
               ScalarValue value() const { return *value_ptr; }
               ScalarValue operator*() const { return value(); }

               // This interface can be used to probe vector properties
               size_t size() const { return *value_ptr; }
               ScalarValue operator[](const size_t index) const { return vector_data_ptr[index]; }
               const ScalarValue * begin() const { return vector_data_ptr; }
               const ScalarValue * end() const { return begin() + size(); }

            private:
               const PropertyName property_name;
               const PropertyTag tag;
               const PropertyType * const value_ptr;
               const PropertyType * const vector_data_ptr;
         };

         // Properties can be iterated through using range-based for loops
         class const_iterator {
            public:
               // Iterators are just sophisticated pointers to the internal list data
               const_iterator(const PropertyType * target) : internal_ptr{target} {}

               // These operators, together, implement the level of iterator support required for range iteration
               const_iterator & operator++();
               bool operator==(const const_iterator & ref) { return (internal_ptr == ref.internal_ptr); }
               bool operator!=(const const_iterator & ref) { return !(*this == ref); }
               PropertyView operator*() { return PropertyView(internal_ptr); }

            private:
               const PropertyType * internal_ptr;
         };

         // It is easy to append new property values, either using STL containers or OpenCL's native zero-terminated format
         void append(const PropertyName name, const ScalarValue value);
         void append(const PropertyName name, const std::vector<ScalarValue> & vector_value);
         size_t append(const PropertyName name, const ScalarValue * const opencl_vector_value); // Returns the size of the input vector

         // Trying to access a value leads a read-only view of it (see above).
         // WARNING: This operation requires a linear search, prefer iteration for efficient parsing.
         PropertyView operator[](const PropertyName name) const;

         // It is also possible to iterate through the views using range-based iteration
         const_iterator begin() const { return const_iterator(internal_storage_ptr()); }
         const_iterator end() const { return const_iterator(internal_storage_ptr() + internal_storage.size()); }

         // Finally, property lists can be converted to OpenCL-style zero-terminated lists for final input.
         // NOTE: Because OpenCL expects a pointer, this requires keeping the output as mutable internal state.
         const PropertyType * opencl_view();

      private:
         std::vector<PropertyType> internal_storage;
         const PropertyType * internal_storage_ptr() const { return &internal_storage[0]; }

         std::vector<PropertyType> opencl_compatible_view;

         // Determine the length of an OpenCL-style zero-terminated vector (trailing zero NOT included)
         static size_t opencl_vector_size(const ScalarValue * const opencl_vector_value);

         // Reserve enough internal storage to store N extra elements
         void reserve_extra_elems(std::vector<PropertyType> & vector, const size_t extra_elements) { vector.reserve(vector.size() + extra_elements); }
         
         // Property tag identification relies on an internal database of OpenCL properties, so it can fail.
         // In this case, UnsupportedProperty will be thrown.
         static PropertyTag find_property_tag(const PropertyName name);

   };

   namespace UnitTests {

      // Test the components of this unit
      void run_tests_property_list();

   }

}

#endif
