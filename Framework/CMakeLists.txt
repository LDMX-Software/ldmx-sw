# find ROOT first so variables are usable in module declaration
find_package(ROOT REQUIRED COMPONENTS Core RIO)

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

# declare Event module
module(
  NAME Framework
  EXTRA_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  EXTRA_LINK_LIBRARIES ${ROOT_LIBRARIES} ${PYTHON_LIBRARIES} dl
  EXECUTABLES src/ldmx-app.cxx
  DEPENDENCIES Event
)

# include ROOT CMake macros and compilation settings
include(${ROOT_USE_FILE})



# install ROOT pcm file
install(FILES  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# copy Python wrapper to installation dir
install(FILES ${PROJECT_SOURCE_DIR}/python/ldmxcfg.py DESTINATION python/Framework)

