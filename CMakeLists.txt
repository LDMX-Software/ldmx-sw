# minimum cmake version
cmake_minimum_required(VERSION 3.0)

# find ROOT installation
find_package(ROOT REQUIRED COMPONENTS Core RIO PyROOT Geom)
if(ROOT_FOUND)
  message(STATUS "ROOT found at '${ROOT_DIR}'")
endif()

# find XercesC installation
if(XercesC_DIR)
  set(XercesC_INCLUDE_DIR "${XercesC_DIR}/include")
  set(XercesC_LIBRARIES "${XercesC_DIR}/lib/libxerces-c.so")
  if (NOT EXISTS "${XercesC_LIBRARIES}")
    message(FATAL_ERROR "Unable to guess location of Xerces lib from XercesC_DIR.  Please specify using '-DXercesC_LIBRARIES'.")
  endif()
  message(STATUS "Set XercesC_INCLUDE_DIR='${XercesC_INCLUDE_DIR}' and XercesC_LIBRARIES='${XercesC_LIBRARIES}' from Xerces_DIR='${Xerces_DIR}'")
endif()
find_package(XercesC REQUIRED)
if(XercesC_FOUND)
  message(STATUS "XercesC found with lib '${XercesC_LIBRARIES}' and include dir '${XercesC_INCLUDE_DIRS}'")
  get_filename_component(XercesC_LIBRARY_DIR ${XercesC_LIBRARIES} DIRECTORY)
endif() 

# find Geant4 installation
find_package(Geant4 REQUIRED ui_all vis_all)
if(Geant4_FOUND)
  message(STATUS "Geant4 found at '${Geant4_DIR}'")
endif()

# find Python installation
find_package(PythonLibs REQUIRED)
if(PYTHONLIBS_FOUND)
  message(STATUS "PythonLibs found with lib '${PYTHON_LIBRARIES}' and include dir '${PYTHON_INCLUDE_DIRS}'")
  get_filename_component(PYTHON_LIBRARY_DIR ${PYTHON_LIBRARIES} DIRECTORY)
endif()

# mag field map installation
option(INSTALL_FIELDMAP "Install field map data from MagFieldMap sub-module" ON)
if(INSTALL_FIELDMAP)
  message(STATUS "Field map will be installed.")
  install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Scripts/InstallMagFieldMap.cmake")
else()
  message(STATUS "Field map installation disabled.")
endif()

# set C++ options
#set(BASE_CXX_FLAGS "-pipe -m64 -fsigned-char -fPIC -pthread -std=c++11 -fPIC -Wall -pedantic")

# option to print extra module information during CMake config
option(MODULE_DEBUG "Print extra module information during CMake config" OFF)

# add dir with extra CMake modules 
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules/)

# import macro for declaring modules
include(MacroModule)

include(MacroExtDeps)

# declare list of modules with correct dependency order
set(MODULES Event Framework DetDescr EventProc SimPlugins Biasing SimApplication Configuration Detectors)

# build each module in the list
foreach(module ${MODULES})
  message(STATUS "Adding module '${module}'")
  add_subdirectory(${module})
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
    install(CODE "execute_process(COMMAND doxygen ${PROJECT_SOURCE_DIR}/docs/doxygen.conf)")

    # documentation generation target 
    add_custom_target(doc COMMAND doxygen ${PROJECT_SOURCE_DIR}/docs/doxygen.conf)

else()
    # message that documentation is off for this build
    message(STATUS "Generation of Doxygen documentation is disabled.")    
endif()

option(DUMP_CMAKE_VARIABLES OFF)
if(DUMP_CMAKE_VARIABLES)
  get_cmake_property(_variableNames VARIABLES)
  foreach(_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
endif()
