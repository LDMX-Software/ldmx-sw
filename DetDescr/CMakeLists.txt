# project name
project(DetDescr CXX)

# set include dir var
SET(DetDescr_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

# export include dir var so dependent modules can see it
SET(DetDescr_INCLUDE_DIR ${DetDescr_INCLUDE_DIR} PARENT_SCOPE)

# add include dir to build
include_directories(${DetDescr_INCLUDE_DIR})

# set library install dir for package config
set(DetDescr_LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)

# list sources for building the application
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${DetDescr_INCLUDE_DIR}/DetDescr/include/*.h)

# create library including ROOT dict
add_library(${PROJECT_NAME} SHARED ${sources})

# generate package configuration
generate_package_configuration_files(DetDescrConfig.cmake DetDescrConfigVersion.cmake)

# install library
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# test program 
add_executable(ldmx-detector-id-test test/detector_id_test.cc)
target_link_libraries(ldmx-detector-id-test ${PROJECT_NAME})
install(TARGETS ldmx-detector-id-test DESTINATION bin)
