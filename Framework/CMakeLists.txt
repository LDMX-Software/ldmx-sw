# find ROOT first so variables are usable in module declaration
find_package(ROOT REQUIRED COMPONENTS Core RIO)

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

# declare Event module
module(
  NAME Framework
  EXTRA_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  EXTRA_LINK_LIBRARIES ${ROOT_LIBRARIES} ${PYTHON_LIBRARIES} dl
  EXECUTABLES src/ldmxsw.cxx
  DEPENDENCIES Event
)

# include ROOT CMake macros and compilation settings
include(${ROOT_USE_FILE})

# generate ROOT dictionary for Event classes
#root_generate_dictionary(EventDict ${Event_INCLUDE_DIR}/Event/EventDef.h MODULE ${PROJECT_NAME} LINKDEF ${Event_INCLUDE_DIR}/Event/EventLinkDef.h)

# install ROOT pcm file
install(FILES  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
