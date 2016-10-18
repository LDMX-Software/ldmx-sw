# minimum cmake version
cmake_minimum_required(VERSION 3.6)

# add dir with extra CMake modules 
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules/)

# import macro for declaring modules
include(MacroModule)

# declare list of modules with correct dependency order
set(MODULES Event DetDescr SimApplication)

# build each module in the list
foreach(module ${MODULES})
  message(STATUS "Adding module '${module}'")
  add_subdirectory(${module})
  message(STATUS "")
endforeach()
