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

namespace CLplusplus {

   template<typename PropertyType>
   PropertyList<PropertyType>::PropertyList(const PropertyType * const opencl_property_list) {
      // If provided with a null pointer, tell the caller to get lost
      if(opencl_property_list == nullptr) throw InvalidArgument{};
      
      // Decode the OpenCL property list into a less dangerous internal format
      const PropertyType * input_parser = opencl_property_list;
      PropertyName current_property = *input_parser;
      while(current_property != 0) {
         // Determine if the property currently being scanned is a scalar or a vector
         const PropertyTag property_tag = find_property_tag(current_property);
         ++input_parser;

         // Dispatch property processing to the appropriate append function
         switch(property_tag) {
            case PropertyTag::Scalar:
               // Scalar inputs are very easy to handle
               append(current_property, *input_parser);
               ++input_parser;
               break;
            case PropertyTag::Vector:
               // Vector inputs require some size tracking so that we can update the input parser
               {
                  const size_t vector_size = append(current_property, input_parser);
                  input_parser += 1 + vector_size;
               }
               break;
            default:
               // Abort parsing upon encountering an unsupported property type
               throw UnsupportedProperty();
         }
         
         // Fetch next property name
         current_property = *input_parser;
      }
   }

   template<typename PropertyType>
   typename PropertyList<PropertyType>::const_iterator & PropertyList<PropertyType>::const_iterator::operator++() {
      // Analyze the property value we are pointing to
      PropertyView target = **this;

      // Advance the internal pointer accordingly
      internal_ptr += 2; // Property name and tag
      switch(target.kind()) {
         case PropertyTag::Scalar:
            internal_ptr += 1; // Scalar value
            break;
         case PropertyTag::Vector:
            internal_ptr += 1 + target.size(); // Vector length and data
            break;
         default:
            throw UnsupportedProperty{};
      }

      // Update our iterator information and return it
      return *this;
   }

   template<typename PropertyType>
   void PropertyList<PropertyType>::append(const PropertyName name, const ScalarValue value) {
      // For a scalar property, we need to store the property name, the Scalar tag, and the value
      reserve_extra_elems(internal_storage, 3);

      // Insert the property
      internal_storage.push_back(name);
      internal_storage.push_back(static_cast<PropertyType>(PropertyTag::Scalar));
      internal_storage.push_back(value);
   }

   template<typename PropertyType>
   void PropertyList<PropertyType>::append(const PropertyName name, const std::vector<ScalarValue> & vector_value) {
      // For a vector property, we need to store the property name, the Vector tag, the vector size, and the values
      reserve_extra_elems(internal_storage, 3 + vector_value.size());

      // Insert the property
      internal_storage.push_back(name);
      internal_storage.push_back(static_cast<PropertyType>(PropertyTag::Vector));
      internal_storage.push_back(vector_value.size());
      for(const auto & value : vector_value) {
         internal_storage.push_back(value);
      }
   }

   template<typename PropertyType>
   size_t PropertyList<PropertyType>::append(const PropertyName name, const ScalarValue * const opencl_vector_value) {
      // Determine the size of the OpenCL vector value
      const size_t vector_value_size = opencl_vector_size(opencl_vector_value);

      // Reserve storage for the vector property (as above)
      reserve_extra_elems(internal_storage, 3 + vector_value_size);

      // Insert the property
      internal_storage.push_back(name);
      internal_storage.push_back(static_cast<PropertyType>(PropertyTag::Vector));
      internal_storage.push_back(vector_value_size);
      for(size_t i = 0; i < vector_value_size; ++i) {
         internal_storage.push_back(opencl_vector_value[i]);
      }
      
      // Return the input vector's size
      return vector_value_size;
   }

   template<typename PropertyType>
   typename PropertyList<PropertyType>::PropertyView PropertyList<PropertyType>::operator[](const PropertyName name) const {
      // Iterate through the property list until the right value is found (there is really nothing better to do :-()
      for(const auto & property : *this) {
         if(property.name() == name) return property;
      }

      // If we don't find the property in the list, the caller must have done something stupid
      throw InvalidArgument{};
   }

   template<typename PropertyType>
   const PropertyType * PropertyList<PropertyType>::opencl_view() {
      // Initialize output storage
      opencl_compatible_view.clear();

      // Start parsing our internal property list
      size_t current_index = 0;
      while(current_index < internal_storage.size()) {
         // Fetch and append next property's name
         const PropertyName property_name = internal_storage[current_index];
         opencl_compatible_view.push_back(property_name);
         ++current_index;

         // Choose a course of action depending on value tag
         const auto property_tag = static_cast<PropertyTag>(internal_storage[current_index]);
         ++current_index;
         switch(property_tag) {
            case PropertyTag::Scalar:
               // Fetch and append scalar value
               opencl_compatible_view.push_back(internal_storage[current_index]);
               ++current_index;
               break;
            case PropertyTag::Vector:
               {
                  // Fetch vector length
                  const size_t vector_length = internal_storage[current_index];
                  ++current_index;

                  // Append zero-terminated vector to output
                  reserve_extra_elems(opencl_compatible_view, vector_length + 1);
                  for(size_t offset = 0; offset < vector_length; ++offset) {
                     opencl_compatible_view.push_back(internal_storage[current_index + offset]);
                  }
                  opencl_compatible_view.push_back(0);
                  current_index += vector_length;
               }
               break;
            default:
               // Abort parsing upon encountering an unsupported property type
               throw UnsupportedProperty{};
         }
      }

      // Append trailing zero and return OpenCL-compatible parameter list
      opencl_compatible_view.push_back(0);
      return &opencl_compatible_view[0];
   }

   template<typename PropertyType>
   size_t PropertyList<PropertyType>::opencl_vector_size(const ScalarValue * const opencl_vector_value) {
      const ScalarValue * vector_value_end = opencl_vector_value;
      while(*vector_value_end != 0) ++vector_value_end;
      return (vector_value_end - opencl_vector_value);
   }

   template<typename PropertyType>
   typename PropertyList<PropertyType>::PropertyTag PropertyList<PropertyType>::find_property_tag(const PropertyName name) {
      switch(name) {
         #ifdef CL_VERSION_1_2
         case CL_DEVICE_PARTITION_EQUALLY: return PropertyTag::Scalar;
         case CL_DEVICE_PARTITION_BY_COUNTS: return PropertyTag::Vector;
         case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN: return PropertyTag::Scalar;
         #endif

         case CL_CONTEXT_PLATFORM: return PropertyTag::Scalar;
         #ifdef CL_VERSION_1_2
         case CL_CONTEXT_INTEROP_USER_SYNC: return PropertyTag::Scalar;
         #endif

         // WARNING : This database must be kept up to date as new properties are added to OpenCL

         default: throw UnsupportedProperty{};
      }
   }

}
