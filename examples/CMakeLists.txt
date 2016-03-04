#===============================
# Build the CLplusplus examples
#===============================

# Use the example-specific headers
INCLUDE_DIRECTORIES( include/ )

# Copy the OpenCL kernels into the build directory
FILE( COPY src/kernels
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )

# Enumerate the examples
FILE( GLOB EXAMPLES_SRC src/*.cpp )
FOREACH( EXAMPLE_SRC ${EXAMPLES_SRC} )
    # Find the executable name
    GET_FILENAME_COMPONENT( EXAMPLE_EXE ${EXAMPLE_SRC} NAME_WE )
    
    # Build it, linking with the main library in the way
    ADD_EXECUTABLE( ${EXAMPLE_EXE} ${EXAMPLE_SRC} )
    TARGET_LINK_LIBRARIES( ${EXAMPLE_EXE}
                           PRIVATE ${CMAKE_PROJECT_NAME} )
                           
    # Mark it as a unit test for the library
    ADD_TEST( NAME examples-${EXAMPLE_EXE}
              COMMAND ${EXAMPLE_EXE} )
ENDFOREACH( EXAMPLE_SRC )
