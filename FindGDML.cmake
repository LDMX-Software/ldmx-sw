#
# FindGDML.cmake
#
# Finds the ONNX Runtime binaries or download them if a suitable version isn't
# found.
#
# This will define the following variables
#
# GDML_FOUND GDML_INCLUDE_DIRS - path to headers GDML_LIBRARIES    - path to
# libraries
#
# and the following targets
#
# GDML::GDML
#

if(NOT TARGET GDML::GDML)

  find_path(
    gdml_include_dir
    NAMES SAXProcessor.h
    PATHS ${GDML_DIR}/include
    PATH_SUFFIXES Saxana)

  # If GDML can't be found, throw a fatal error.
  if(NOT gdml_include_dir)
    message(
      FATAL_ERROR
        "GDML was not found. Specify the install directory by setting GDML_DIR."
    )
  endif()

  # Create the target
  add_library(GDML::GDML SHARED IMPORTED GLOBAL)
  set_target_properties(
    GDML::GDML PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GDML_DIR}/include"
                          IMPORTED_LOCATION "${GDML_DIR}/lib/libgdml.so")

  message(STATUS "Found GDML at ${GDML_DIR}")
  set(GDML_FOUND TRUE)

endif()
