#======================================================================
# Automatically generate the project's documentation using asciidoctor
#======================================================================

# Locate the asciidoctor executable which we'll use to generate the documentation
FIND_PROGRAM( ASCIIDOCTOR_EXECUTABLE asciidoctor )
if( ${ASCIIDOCTOR_EXECUTABLE} STREQUAL "ASCIIDOCTOR_EXECUTABLE-NOTFOUND" )

    # Abort with a warning if we can't find asciidoctor
    MESSAGE( WARNING "asciidoctor was not found, documentation will not be generated" )
    
else()

    # Define the documentation source files and output
    SET( DOC_INDEX "${CMAKE_CURRENT_SOURCE_DIR}/index.adoc" )
    FILE( GLOB DOC_SRC "*.adoc" )
    SET( DOC_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/index.html" )

    # Instruct CMake how to build the documentation
    ADD_CUSTOM_COMMAND( OUTPUT ${DOC_OUTPUT}
                        COMMAND ${ASCIIDOCTOR_EXECUTABLE} -v ${DOC_INDEX} -o ${DOC_OUTPUT}
                        MAIN_DEPENDENCY ${DOC_INDEX}
                        DEPENDS ${DOC_SRC}
                        COMMENT "Rendering the documentation"
                        VERBATIM )

    # Add a an associated target and make sure it is built by default
    ADD_CUSTOM_TARGET( doc
                       ALL
                       DEPENDS ${DOC_OUTPUT} )
    
    # Make sure that the doc will be installed in the end
    INSTALL( DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
             DESTINATION share/doc/${CMAKE_PROJECT_NAME} )

endif()
