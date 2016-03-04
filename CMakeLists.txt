#==============================
# Build everything using CMake
#==============================

# It is wise to ask for a minimal CMake release and to make sure we don't go beyond its capabilities
CMAKE_MINIMUM_REQUIRED( VERSION 3.1 )
CMAKE_POLICY( VERSION 3.1 )

# We should also check for OpenCL support early on
ENABLE_LANGUAGE( C )
FIND_PACKAGE( OpenCL 1.2 REQUIRED )

# Beyond that, well, this is the CMake build system for CLplusplus, a C++11 library based on OpenCL
PROJECT( CLplusplus LANGUAGES CXX )
SET( CMAKE_CXX_STANDARD 11 )
SET( CMAKE_CXX_STANDARD_REQUIRED ON )
INCLUDE_DIRECTORIES( ${OpenCL_INCLUDE_DIRS} )
LINK_LIBRARIES( ${OpenCL_LIBRARIES} )

# Make sure that the main library headers will be available during the build
INCLUDE_DIRECTORIES( "include" )

# Enable CTest
ENABLE_TESTING()

# Build the CLplusplus components
ADD_SUBDIRECTORY( doc )
ADD_SUBDIRECTORY( examples )
ADD_SUBDIRECTORY( include )
ADD_SUBDIRECTORY( src )
