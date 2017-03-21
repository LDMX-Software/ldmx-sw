# minimum cmake version
cmake_minimum_required(VERSION 3.0)

# find ROOT installation
find_package(ROOT REQUIRED COMPONENTS Core RIO PyROOT Geom)
if (ROOT_FOUND)
  message(STATUS "ROOT found at '${ROOT_DIR}'")
else()
  message(FATAL_ERROR "ROOT installation was not found.")
endif()

# find Geant4 installation
find_package(Geant4 REQUIRED ui_all vis_all)
if (Geant4_FOUND)
  message(STATUS "Geant4 found at '${Geant4_DIR}'")
else()
  message(FATAL_ERROR "Geant4 installation was not found.")
endif()

# find Python installation
find_package(PythonLibs REQUIRED)
if (PYTHONLIBS_FOUND)
  message(STATUS "PythonLibs found with lib '${PYTHON_LIBRARIES}' and include dir '${PYTHON_INCLUDE_DIRS}'")
  get_filename_component(PYTHON_LIBRARY_DIR ${PYTHON_LIBRARIES} DIRECTORY)
else()
  message(FATAL_ERROR "PythonLibs was not found.")
endif()

# mag field map installation
option(INSTALL_FIELDMAP "Install field map data from MagFieldMap sub-module" ON)
if (INSTALL_FIELDMAP)
  message(STATUS "Field map will be installed.")
  install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Scripts/InstallMagFieldMap.cmake")
else()
  message(STATUS "Field map installation disabled.")
endif()

# option to print extra module information during CMake config
option(MODULE_DEBUG "Print extra module information during CMake config" OFF)

# add dir with extra CMake modules 
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules/)

# import macro for declaring modules
include(MacroModule)

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
    install(CODE "EXECUTE_PROCESS(COMMAND doxygen ${PROJECT_SOURCE_DIR}/docs/doxygen.conf)")

    # documentation generation target 
    add_custom_target(doc COMMAND doxygen ${PROJECT_SOURCE_DIR}/docs/doxygen.conf)

else()
    # message that documentation is off for this build
    message(STATUS "INSTALL_DOC is set to OFF.  Documentation will not be created.")    
endif()
