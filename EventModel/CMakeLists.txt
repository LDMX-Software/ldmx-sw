# minimum cmake version
cmake_minimum_required(VERSION 3.6)

# project name
project(EventModel CXX)

# project version
SET(EventModel_VERSION_MAJOR 1)
SET(EventModel_VERSION_MINOR 0)
SET(EventModel_VERSION_PATCH 0)
set(EventModel_VERSION "${EventModel_VERSION_MAJOR}.${EventModel_VERSION_MINOR}.${EventModel_VERSION_PATCH}" )

# allows calling export_library_dependencies used in package config file generation
cmake_policy(SET CMP0033 OLD)

# define path to extra CMake modules
set(LDMX_CMAKE_MODULES_ROOT ${PROJECT_SOURCE_DIR}/../cmake/Modules/)

# add dir with extra CMake modules 
list(APPEND CMAKE_MODULE_PATH ${LDMX_CMAKE_MODULES_ROOT})

# macro for generating package config files
include(MacroGeneratePackageConfigFiles)

# add project include dir
SET(EventModel_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
include_directories(${EventModel_INCLUDE_DIR})

# set library dir 
# FIXME: This should be set automatically but is not!
SET(EventModel_LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)

# list sources for building the application
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h)

# create shared library
add_library( EventModel SHARED ${sources} )

# generate package configuration
generate_package_configuration_files(EventModelConfig.cmake EventModelConfigVersion.cmake EventModelLibDeps.cmake)

# install library
install(TARGETS EventModel DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
