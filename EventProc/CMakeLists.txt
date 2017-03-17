# find ROOT
find_package(ROOT REQUIRED COMPONENTS Core RIO TreePlayer PyROOT)

# declare SimApplication module
module(
  NAME EventProc
  EXTRA_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  EXTRA_LINK_LIBRARIES ${ROOT_LIBRARIES} DetDescr
  DEPENDENCIES Event DetDescr Framework
)

# include ROOT CMake macros and compilation settings
include(${ROOT_USE_FILE})
target_link_libraries(DetDescr)
 
