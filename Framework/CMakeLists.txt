# declare Event module
module(
  NAME Framework
  EXTRA_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS}
  EXTRA_LINK_LIBRARIES ${ROOT_LIBRARIES} ${PYTHON_LIBRARIES} dl
  EXECUTABLES src/ldmx-app.cxx
  DEPENDENCIES Event
)

# include ROOT CMake macros and compilation settings
include(${ROOT_USE_FILE})
