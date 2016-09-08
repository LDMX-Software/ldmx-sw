# project name
project(Event CXX)

# project version
set(Event_VERSION_MAJOR 1)
set(Event_VERSION_MINOR 0)
set(Event_VERSION_PATCH 0)

# find ROOT
find_package(ROOT REQUIRED COMPONENTS Core RIO)

# include ROOT macros and functions
include(${ROOT_USE_FILE})

# add project include dir
SET(Event_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
include_directories(${Event_INCLUDE_DIR})

# add ROOT include dirs
include_directories(${ROOT_INCLUDE_DIRS})

# set library dir 
# FIXME: This should be set automatically by one of the macros but it is not!
set(Event_LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib)

# list sources for building the application
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${Event_INCLUDE_DIR}/Event/include/*.h)

# generate ROOT dict
ROOT_GENERATE_DICTIONARY(EventDict ${Event_INCLUDE_DIR}/Event/Event.h MODULE ${PROJECT_NAME} LINKDEF ${Event_INCLUDE_DIR}/Event/EventLinkDef.h)

# create library including ROOT dict
add_library(${PROJECT_NAME} SHARED ${sources} EventDict.cxx)
target_link_libraries(${PROJECT_NAME} ${ROOT_LIBRARIES})

# test program
add_executable(ldmx-event-model-test test/ldmx_event_model_test.cc)
target_link_libraries(ldmx-event-model-test ${ROOT_LIBRARIES} ${PROJECT_NAME})

# generate package configuration
generate_package_configuration_files(EventConfig.cmake EventConfigVersion.cmake)
# EventLibDeps.cmake

# install library
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# install test program
install(TARGETS ldmx-event-model-test DESTINATION bin)
