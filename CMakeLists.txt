# minimum cmake version
cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
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
find_package(PythonLibs 2 EXACT REQUIRED)
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

# get list module names 
# a directory in the CMAKE_CURRENT_SOURCE_DIR is considered a module if
#  something named "include", "src", and "CMakeLists.txt" all exists within it
set( UNSORTED "" )
file( GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/* )
foreach( child ${children} )
  if ( (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${child}/include")
   AND (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${child}/src")
   AND (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${child}/CMakeLists.txt" ) )
    #child is correct format to be a module
    list( APPEND UNSORTED ${child} )
  endif()
endforeach()

# Sort modules so that each item has its dependencies listed before it.
# A modules internal dependencies are parsed from its CMakeLists.txt file.
# This will turn in to an infinite loop if there are ANY circular dependencies
#  listed in the CMakeLists.txt files (even if they wouldn't cause a
#  compilation error). Infinite loops are prevented by a FATAL_ERROR.
set( MODULES "" )
list( LENGTH UNSORTED nleft )
while( nleft GREATER 0 )
  #loop until there are no modules left in UNSORTED
  set( addedmodule FALSE )
  foreach( module ${UNSORTED} )
    # get dependency list
    file( STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/${module}/CMakeLists.txt" deps REGEX ".* DEPENDENCIES.*" )
    
    # check if dependencies (if they exist) have already been listed
    set( nodeps TRUE )
    if( NOT( deps STREQUAL "") )
      string( REPLACE " " ";" deps ${deps} )
      #look for dependencies that haven't been sorted yet
      foreach( dep ${deps} )
        list( FIND UNSORTED ${dep} index )
        if( index GREATER -1 )
          #module has dependency that hasn't been listed before it
          set( nodeps FALSE )
          break()
        endif()
      endforeach()
    endif() 
    
    #if module had no dependencies already listed
    if( nodeps )
      list( APPEND MODULES ${module} )
      list( REMOVE_ITEM UNSORTED ${module} )
      set( addedmodule TRUE )
    endif()

  endforeach() #loop through modules in UNSORTED

  list( LENGTH UNSORTED nleft ) #reset number of UNSORTED left to sort

  # infinite loop prevention
  #  if nothing was added on this loop, then nothing will be added on next
  #  loop because conditions didn't change
  if ( NOT(addedmodule) )
    message( FATAL_ERROR "Infinite loop when creating module list. Check for circular dependencies in module CMakeLists.txt" )
  endif()

endwhile() #loop until no modules left in UNSORTED

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

# build Detectors module
add_subdirectory(Detectors)

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
