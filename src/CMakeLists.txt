#=====================================
# Build the CLplusplus library itself
#=====================================

# Every file in this directory is part of the CLplusplus library source
FILE( GLOB LIBRARY_SRC *.cpp )

# For files that belong to the library, it is unnecessary to prefix header paths
INCLUDE_DIRECTORIES( ../include/${CMAKE_PROJECT_NAME} )

# Build the CLplusplus library from this source
ADD_LIBRARY( ${CMAKE_PROJECT_NAME}
             SHARED
             ${LIBRARY_SRC} )

# Make sure it is installed wherever appropriate
INSTALL( TARGETS ${CMAKE_PROJECT_NAME}
         DESTINATION lib/ )
