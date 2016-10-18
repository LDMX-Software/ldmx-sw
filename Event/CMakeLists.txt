# find ROOT first so variables are usable in module declaration
find_package(ROOT REQUIRED COMPONENTS Core RIO)

# declare Event module
module(
  NAME Event
  EXTRA_SOURCES EventDict.cxx
  EXTRA_LINK_LIBRARIES ${ROOT_LIBRARIES}
  EXTRA_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
)

# include ROOT CMake macros and compilation settings
include(${ROOT_USE_FILE})

# generate ROOT dictionary for Event classes
root_generate_dictionary(EventDict ${Event_INCLUDE_DIR}/Event/Event.h MODULE ${PROJECT_NAME} LINKDEF ${Event_INCLUDE_DIR}/Event/EventLinkDef.h)

# install ROOT pcm file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)