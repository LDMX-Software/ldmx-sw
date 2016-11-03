# minimum cmake version
cmake_minimum_required(VERSION 3.6)

# add dir with extra CMake modules 
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules/)

# import macro for declaring modules
include(MacroModule)

# declare list of modules with correct dependency order
set(MODULES Event DetDescr EventProc SimPlugins SimApplication)

# build each module in the list
foreach(module ${MODULES})
  message(STATUS "Adding module '${module}'")
  add_subdirectory(${module})
  message(STATUS "")
endforeach()

# create env setup script
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/ldmx-setup-env.sh.in ${CMAKE_CURRENT_BINARY_DIR}/ldmx-setup-env.sh)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ldmx-setup-env.sh DESTINATION bin)
