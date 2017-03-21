# declare DetDescr module
module(
  NAME DetDescr
  EXTRA_LINK_LIBRARIES ${ROOT_LIBRARIES}
  EXTRA_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
)
# include ROOT CMake macros and compilation settings
include(${ROOT_USE_FILE})
