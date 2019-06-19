# minimum cmake version
cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 14)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


# find XercesC installation
if(XercesC_DIR)
  set(XercesC_INCLUDE_DIR "${XercesC_DIR}/include")
  set(XercesC_LIBRARY "${XercesC_DIR}/lib/libxerces-c.so")
  if (NOT EXISTS "${XercesC_LIBRARY}")
    message(FATAL_ERROR "Unable to guess location of Xerces lib.  Please specify using: -DXercesC_LIBRARY")
  endif()
  message(STATUS "XercesC dir set to: ${XercesC_DIR}")
  message(STATUS "XercesC include dir set to: ${XercesC_INCLUDE_DIR}")
  message(STATUS "XercesC library set to: ${XercesC_LIBRARY}")
endif()
find_package(XercesC REQUIRED)
message(STATUS "XercesC libs found at: ${XercesC_LIBRARIES}")
message(STATUS "XercesC include dir found at: ${XercesC_INCLUDE_DIRS}")
get_filename_component(XercesC_LIBRARY_DIR ${XercesC_LIBRARIES} DIRECTORY)

# find Geant4 installation
find_package(Geant4 REQUIRED ui_all vis_all)
message(STATUS "Geant4 found at: ${Geant4_DIR}")

# find Python installation
find_package(PythonLibs REQUIRED)
message(STATUS "Python lib found at: ${PYTHON_LIBRARIES}")
message(STATUS "Python include dir found at: ${PYTHON_INCLUDE_DIRS}")
get_filename_component(PYTHON_LIBRARY_DIR ${PYTHON_LIBRARIES} DIRECTORY)

# find ROOT installation
find_package(ROOT REQUIRED COMPONENTS Core RIO PyROOT Geom Eve Gui)
message(STATUS "ROOT found at: ${ROOT_DIR}")

# find boost headers
# specify location with -DBOOST_ROOT=<location-of-boost-prefix>
find_package(Boost REQUIRED)
message(STATUS "Boost lib found at: ${Boost_LIBRARY_DIRS}")
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

# mag field map installation
option(INSTALL_FIELDMAP "Install field map data from MagFieldMap sub-module" ON)
message(STATUS "MagFieldMap installation: ${INSTALL_FIELDMAP}")
if(INSTALL_FIELDMAP)
  install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Scripts/InstallMagFieldMap.cmake")
endif()

# option to print extra module information during CMake config
option(MODULE_DEBUG "Print extra module information during CMake config" OFF)

# add dir with extra CMake modules 
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules/)

# import macro for declaring modules
include(MacroModule)

# import macro for declaring external dependencies
include(MacroExtDeps)

# declare list of modules with correct dependency order
set(MODULES Event Framework DetDescr Tools EventProc SimCore SimPlugins Biasing SimApplication Configuration Detectors Ecal EventDisplay DQM)

# build each module in the list
foreach(module ${MODULES})
  message(STATUS "Adding module: ${module}")
  add_subdirectory(${module})
  if(EXTRA_INC)
      message(STATUS "Adding include directory: ${EXTRA_INC}")
      include_directories(${EXTRA_INC}) 
      unset(EXTRA_INC)
  endif()
endforeach()

# configure scripts and copy to installation dir 
file(GLOB config_scripts ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.in)
foreach(config_script ${config_scripts})
  string(REPLACE ".in" "" config_script_output ${config_script})
  get_filename_component(config_script_output ${config_script_output} NAME)
  configure_file(${config_script} ${CMAKE_CURRENT_BINARY_DIR}/${config_script_output})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${config_script_output} DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endforeach()

# install python init file for top-level LDMX module
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py "# python package")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py DESTINATION lib/python/LDMX)

# install scripts to bin dir
file(GLOB script_files ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.sh ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.py)
foreach(script_file ${script_files}) 
    install(FILES ${script_file} DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endforeach()

# configure and generate documentation using doxygen
option(INSTALL_DOC "Set to ON to generate documentation using doxygen" OFF)
message(STATUS "Doxygen documentation: ${INSTALL_DOC}")
if(INSTALL_DOC)

    # message that documentation is off for this build
    message(STATUS "Doxygen documentation will be generated")

    # find doxygen
    find_program(DOXYGEN_EXECUTABLE doxygen ${PATH})
    if(DOXYGEN_EXECUTABLE-NOTFOUND)
        message(FATAL_ERROR "The doxygen executable was not found")
    endif()

    # find dot
    #find_program(DOT_EXECUTABLE dot ${PATH})
    #if(DOT_EXECUTABLE-NOTFOUND)
    #    message(FATAL_ERROR "The dot executable was not found.")
    #endif()
    
    # configure doxygen file
    configure_file(${PROJECT_SOURCE_DIR}/docs/doxygen.conf.in ${PROJECT_SOURCE_DIR}/docs/doxygen.conf)

    # generate the documentation
    install(CODE "execute_process(COMMAND doxygen ${PROJECT_SOURCE_DIR}/docs/doxygen.conf WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})")

    # documentation generation target 
    add_custom_target(doc COMMAND doxygen ${PROJECT_SOURCE_DIR}/docs/doxygen.conf)

endif()

# option for dumping full CMake env
option(DUMP_CMAKE_VARIABLES OFF)
if(DUMP_CMAKE_VARIABLES)
  get_cmake_property(_variableNames VARIABLES)
  foreach(_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
endif()

# info message about install prefix
message(STATUS "ldmx-sw will be installed to: ${CMAKE_INSTALL_PREFIX}")
