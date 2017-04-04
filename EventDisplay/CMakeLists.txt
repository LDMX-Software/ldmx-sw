# find ROOT
find_package(ROOT REQUIRED COMPONENTS Core RIO TreePlayer Eve Gui)

# declare SimApplication module
module(
  NAME EventDisplay
  EXTRA_SOURCES EventDisplayDict.cxx
  EXTRA_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  EXTRA_LINK_LIBRARIES ${ROOT_LIBRARIES} DetDescr
  DEPENDENCIES Event DetDescr Framework
)
target_link_libraries(DetDescr)

# include ROOT CMake macros and compilation settings
include(${ROOT_USE_FILE})

# generate ROOT dictionary for Event classes 
root_generate_dictionary(EventDisplayDict ${EventDisplay_INCLUDE_DIR}/EventDisplay/EventDisplayDef.h MODULE ${PROJECT_NAME} LINKDEF ${EventDisplay_INCLUDE_DIR}/EventDisplay/EventDisplayLinkDef.h)

# install ROOT pcm file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

 
