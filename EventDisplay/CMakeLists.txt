# declare EventDisplay module
module(
  NAME EventDisplay
  EXTRA_SOURCES EventDisplayDict.cxx
  DEPENDENCIES Exception Event DetDescr Framework
  EXECUTABLES src/ldmx_eve.cxx
  EXTERNAL_DEPENDENCIES ROOT
)

# generate ROOT dictionary for Event classes 
root_generate_dictionary(EventDisplayDict ${EventDisplay_INCLUDE_DIR}/EventDisplay/EventDisplayDef.h MODULE ${PROJECT_NAME} LINKDEF ${EventDisplay_INCLUDE_DIR}/EventDisplay/EventDisplayLinkDef.h)

# install ROOT pcm file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
