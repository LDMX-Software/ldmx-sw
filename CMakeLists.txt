# minimum cmake version
cmake_minimum_required(VERSION 3.1)

project(ldmx-sw)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT EXISTS /.dockerenv AND NOT EXISTS /singularity)
  message(WARNING "You are not inside a container; you may be working in an untested environment.")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#this piece of cmake nonsense allows us to change the default
#   installation location
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX $ENV{LDMX_BASE}/ldmx-sw/install CACHE PATH "..." FORCE)
endif()

# option to print extra module information during CMake config
option(MODULE_DEBUG "Print extra module information during CMake config" OFF)

# add dir with extra CMake modules 
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules/)

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
find_package(Geant4 REQUIRED ui_all vis_all gdml)
message(STATUS "Geant4 found at: ${Geant4_DIR}")

# find Python installation
find_package(PythonLibs 3)
message(STATUS "Python lib found at: ${PYTHON_LIBRARIES}")
message(STATUS "Python include dir found at: ${PYTHON_INCLUDE_DIRS}")
get_filename_component(PYTHON_LIBRARY_DIR ${PYTHON_LIBRARIES} DIRECTORY)

# find ROOT installation
option(BUILD_EVE "Build the event display application. This requires more components of ROOT." OFF)
if(BUILD_EVE)
    find_package(ROOT 6 REQUIRED COMPONENTS Core RIO Geom Eve Gui)
else()
    find_package(ROOT 6 REQUIRED COMPONENTS Core RIO)
endif()
message(STATUS "ROOT found at: ${ROOT_DIR}")

# find boost headers
# specify location with -DBOOST_ROOT=<location-of-boost-prefix>
find_package(Boost REQUIRED COMPONENTS system log)
message(STATUS "Boost lib found at: ${Boost_LIBRARY_DIRS}")
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# find ONNXRuntime installation
# specify location with -DONNXRUNTIME_ROOT=<location-of-onnxruntime-prefix>
find_package(ONNXRuntime)
if(ONNXRUNTIME_FOUND)
  message(STATUS "ONNXRuntime libs found at: ${ONNXRUNTIME_LIBRARIES}")
  message(STATUS "ONNXRuntime include dir found at: ${ONNXRUNTIME_INCLUDE_DIRS}")
  add_definitions(-DLDMX_USE_ONNXRUNTIME)
else()
  message(WARNING "Cannot find ONNXRuntime: will not be able to run EcalVeto BDT or DNN")
endif()

# mag field map installation
option(INSTALL_FIELDMAP "Install field map data from MagFieldMap sub-module" ON)
message(STATUS "MagFieldMap installation: ${INSTALL_FIELDMAP}")
if(INSTALL_FIELDMAP)
  install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Scripts/InstallMagFieldMap.cmake")
endif()

# import macro for declaring modules
include(MacroModule)

# import macro for declaring external dependencies
include(MacroExtDeps)

# construct list of modules in dependency order
#   puts modules into MODULES cmake variable
include(ConstructModuleList)

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

# build and install test executable ldmx-test
option(BUILD_TESTS "Build and link ldmx-test executable." OFF)
if(BUILD_TESTS)
  include(TestExecutable)
endif()

# install python init file for top-level LDMX module
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py 
    "\"\"\"Python package to configure ldmx-sw\"\"\"")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py DESTINATION lib/python/LDMX)

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
