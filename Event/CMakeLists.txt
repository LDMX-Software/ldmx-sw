# declare Event module
module(
  NAME Event
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  SOURCE_DIR ${PROJECT_SOURCE_DIR}/src
  LIBRARY_NAME Event
  EXTRA_SOURCES EventDict.cxx
)

# find ROOT
find_package(ROOT REQUIRED COMPONENTS Core RIO)

# include ROOT CMake macros and build settings
include(${ROOT_USE_FILE})

# add ROOT include dirs
include_directories(${ROOT_INCLUDE_DIRS})

# generate ROOT dict
root_generate_dictionary(EventDict ${Event_INCLUDE_DIR}/Event/Event.h MODULE ${PROJECT_NAME} LINKDEF ${Event_INCLUDE_DIR}/Event/EventLinkDef.h)

# add ROOT libraries to shared library target
target_link_libraries(${PROJECT_NAME} ${ROOT_LIBRARIES})

# install ROOT pcm file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# TODO: handle test program build with macros

# test program 
add_executable(ldmx-event-model-test test/ldmx_event_model_test.cxx)
target_link_libraries(ldmx-event-model-test ${ROOT_LIBRARIES} ${PROJECT_NAME})
install(TARGETS ldmx-event-model-test DESTINATION bin)

# another test program 
add_executable(ldmx-rand-event-test test/ldmx_rand_event_test.cxx)
target_link_libraries(ldmx-rand-event-test ${ROOT_LIBRARIES} ${PROJECT_NAME})
install(TARGETS ldmx-rand-event-test DESTINATION bin)
