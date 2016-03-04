#==============================
# Build everything using CMake
#==============================

# It is wise to ask for a minimal CMake release and to check for OpenCL support early on
CMAKE_MINIMUM_REQUIRED( VERSION 3.1 )
ENABLE_LANGUAGE( C )
FIND_PACKAGE( OpenCL 1.2 REQUIRED )

# This is the CMake build system for CLplusplus, a C++11 library based on OpenCL
PROJECT( CLplusplus LANGUAGES CXX )
SET( CMAKE_CXX_STANDARD 11 )
SET( CMAKE_CXX_STANDARD_REQUIRED ON )
INCLUDE_DIRECTORIES( ${OpenCL_INCLUDE_DIRS} )
LINK_LIBRARIES( ${OpenCL_LIBRARIES} )

# Make sure that the main library headers will be available during the build
INCLUDE_DIRECTORIES( "include" )

# Enable CTest
ENABLE_TESTING()

# Build the rest
ADD_SUBDIRECTORY( doc )
ADD_SUBDIRECTORY( examples )
ADD_SUBDIRECTORY( include )
ADD_SUBDIRECTORY( src )
