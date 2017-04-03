# declare Event module
module(
  NAME Event
  EXTRA_SOURCES EventDict.cxx
  EXTERNAL_DEPENDENCIES ROOT
)

# generate ROOT dictionary for Event classes
root_generate_dictionary(EventDict ${Event_INCLUDE_DIR}/Event/EventDef.h MODULE ${PROJECT_NAME} LINKDEF ${Event_INCLUDE_DIR}/Event/EventLinkDef.h)

# install ROOT pcm file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
