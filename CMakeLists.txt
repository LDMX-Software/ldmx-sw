# minimum cmake version
cmake_minimum_required(VERSION 3.0)

# add dir with extra CMake modules 
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules/)

# import macro for declaring modules
include(MacroModule)

# declare list of modules with correct dependency order
set(MODULES Event Framework DetDescr EventProc SimCore SimPlugins Biasing SimApplication Configuration)

# build each module in the list
foreach(module ${MODULES})
  message(STATUS "Adding module '${module}'")
  add_subdirectory(${module})
  message(STATUS "")
endforeach()

# create env setup script and copy to install dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/ldmx-setup-env.sh.in ${CMAKE_CURRENT_BINARY_DIR}/ldmx-setup-env.sh)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ldmx-setup-env.sh DESTINATION bin)

# install python init file for top-level LDMX module
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py "# python package")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py DESTINATION python/LDMX)

# configure and generate documentation using doxygen
option(INSTALL_DOC "Set to ON to generate documentation using doxygen." OFF)
if(INSTALL_DOC)

    # find doxygen
    find_program(DOXYGEN_EXECUTABLE doxygen ${PATH})
    if(DOXYGEN_EXECUTABLE-NOTFOUND)
        message(FATAL_ERROR "The doxygen executable was not found.")
    endif()

    # find dot
    #find_program(DOT_EXECUTABLE dot ${PATH})
    #if(DOT_EXECUTABLE-NOTFOUND)
    #    message(FATAL_ERROR "The dot executable was not found.")
    #endif()
    
    # configure doxygen file
    configure_file(${PROJECT_SOURCE_DIR}/docs/doxygen.conf.in ${PROJECT_SOURCE_DIR}/docs/doxygen.conf)

    # generate the documentation
    install(CODE "EXECUTE_PROCESS(COMMAND doxygen ${PROJECT_SOURCE_DIR}/docs/doxygen.conf)")

    # documentation generation target 
    add_custom_target(doc COMMAND doxygen ${PROJECT_SOURCE_DIR}/docs/doxygen.conf)

else()
    # message that documentation is off for this build
    message(STATUS "INSTALL_DOC is set to OFF.  Documentation will not be created.")    
endif()
