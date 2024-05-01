#
# FindLCDD.cmake
#
# Finds the ONNX Runtime binaries or download them if a suitable version isn't
# found.
#
# This will define the following variables
#
# LCDD_FOUND LCDD_INCLUDE_DIRS - path to headers LCDD_LIBRARIES    - path to
# libraries
#
# and the following targets
#
# LCDD::LCDD
#

if(NOT TARGET LCDD::LCDD)

  find_path(
    lcdd_include_dir
    NAMES LCDDParser.hh
    PATHS ${LCDD_DIR}/include
    PATH_SUFFIXES lcdd/core)

  # If LCDD can't be found, throw a fatal error.
  if(NOT lcdd_include_dir)
    message(
      FATAL_ERROR
        "LCDD was not found. Specify the install directory by setting LCDD_DIR."
    )
  endif()

  # Create the target
  add_library(LCDD::LCDD SHARED IMPORTED GLOBAL)
  set_target_properties(
    LCDD::LCDD PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LCDD_DIR}/include"
                          IMPORTED_LOCATION "${LCDD_DIR}/lib/liblcdd.so")

  message(STATUS "Found LCDD at ${LCDD_DIR}")
  set(LCDD_FOUND TRUE)

endif()
