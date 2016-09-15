# project name
project(Event CXX)

# find ROOT
find_package(ROOT REQUIRED COMPONENTS Core RIO)

# include ROOT macros and functions
include(${ROOT_USE_FILE})

# debug mode
set(CMAKE_BUILD_TYPE Debug)

# set include dir var
SET(Event_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

# export include dir var so dependent modules can see it
SET(Event_INCLUDE_DIR ${Event_INCLUDE_DIR} PARENT_SCOPE)

# add include dir to build
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

# another test program 
add_executable(ldmx-rand-event-test test/ldmx_rand_event_test.cc)
target_link_libraries(ldmx-rand-event-test ${ROOT_LIBRARIES} ${PROJECT_NAME})

# generate package configuration
generate_package_configuration_files(EventConfig.cmake EventConfigVersion.cmake)
# EventLibDeps.cmake

# install library
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# install ROOT pcm file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# install test programs
install(TARGETS ldmx-event-model-test DESTINATION bin)
install(TARGETS ldmx-rand-event-test DESTINATION bin)
