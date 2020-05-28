if(BUILD_EVE)
    # declare EventDisplay module
    module(
      NAME EventDisplay
      EXTRA_SOURCES EventDisplayDict.cxx
      DEPENDENCIES Exception Event DetDescr Framework
      EXECUTABLES src/ldmx_eve.cxx
      EXTERNAL_DEPENDENCIES ROOT Python
    )
    
    # generate ROOT dictionary for Event classes
    root_generate_dictionary(${PROJECT_NAME}Dict ${${PROJECT_NAME}_INCLUDE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Def.h LINKDEF ${${PROJECT_NAME}_INCLUDE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}LinkDef.h)
    
    # install ROOT pcm file
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}Dict_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
endif()
