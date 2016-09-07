# minimum cmake version
cmake_minimum_required(VERSION 3.0)

# allows calling export_library_dependencies used in package config file generation
cmake_policy(SET CMP0033 OLD)

# define path to extra CMake modules
set(LDMX_CMAKE_MODULES_ROOT ${PROJECT_SOURCE_DIR}/cmake/Modules/)

# add dir with extra CMake modules 
list(APPEND CMAKE_MODULE_PATH ${LDMX_CMAKE_MODULES_ROOT})

# import utility macros
include(MacroGeneratePackageConfigFiles)
include(MacroCheckPackageLibs)
include(MacroCheckPackageVersion)

# build sub-modules
add_subdirectory(Event)
add_subdirectory(SimApplication)
