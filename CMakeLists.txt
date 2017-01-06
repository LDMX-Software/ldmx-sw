# minimum cmake version
cmake_minimum_required(VERSION 3.0)

# add dir with extra CMake modules 
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules/)

# import macro for declaring modules
include(MacroModule)

# declare list of modules with correct dependency order
set(MODULES Event DetDescr EventProc SimPlugins Biasing SimApplication)

# build each module in the list
foreach(module ${MODULES})
  message(STATUS "Adding module '${module}'")
  add_subdirectory(${module})
  message(STATUS "")
endforeach()

# create env setup script
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/ldmx-setup-env.sh.in ${CMAKE_CURRENT_BINARY_DIR}/ldmx-setup-env.sh)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ldmx-setup-env.sh DESTINATION bin)

# configure and generate documentation using doxygen
option(INSTALL_DOC "Set to OFF to skip generation of documentation using doxygen." ON)
if(INSTALL_DOC)

    # find doxygen
    find_program(DOXYGEN_EXECUTABLE doxygen ${PATH})
    if(DOXYGEN_EXECUTABLE-NOTFOUND)
        message(FATAL_ERROR "The doxygen executable was not found.")
    endif()

    # find dot
    find_program(DOT_EXECUTABLE dot ${PATH})
    if(DOT_EXECUTABLE-NOTFOUND)
        message(FATAL_ERROR "The dot executable was not found.")
    endif()
    
    # find epspdf
    find_program(EPSPDF_EXECUTABLE epspdf ${PATH})
    if(EPSPDF_EXECUTABLE-NOTFOUND)
        message(FATAL_ERROR "The epspdf executable was not found.")
    endif()

    # configure doxygen output
    configure_file(${PROJECT_SOURCE_DIR}/config/doxygen/doxygen.conf.in ${PROJECT_SOURCE_DIR}/config/doxygen/doxygen.conf)

    # generate the documentation
    install(CODE "EXECUTE_PROCESS(COMMAND doxygen ${PROJECT_SOURCE_DIR}/config/doxygen/doxygen.conf)")

else()
    # message that documentation is off for this build
    message(STATUS "INSTALL_DOC is set to OFF.  Documentation will not be created.")    
endif()
