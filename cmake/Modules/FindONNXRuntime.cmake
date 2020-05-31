#[=======================================================================[.rst:
FindONNXRuntime
-----------

Find the ONNX Runtime headers and libraries.

Imported targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` targets:

``ONNXRuntime::ONNXRuntime``
  The ONNXRuntime ``onnxruntime`` library, if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``ONNXRUNTIME_FOUND``
  true if the ONNXRuntime headers and libraries were found
``ONNXRUNTIME_INCLUDE_DIRS``
  the directory containing the ONNXRuntime headers
``ONNXRUNTIME_LIBRARIES``
  ONNXRuntime libraries to be linked

Cache variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``ONNXRUNTIME_INCLUDE_DIR``
  the directory containing the ONNXRuntime headers
``ONNXRUNTIME_LIBRARY``
  the ONNXRuntime library
  
Hints
^^^^^

A user may set ``ONNXRUNTIME_ROOT`` to a ONNXRuntime installation root to 
tell this module where to look.  
#]=======================================================================]

# add the path in the docker container here
set(_ONNXRUNTIME_SEARCHES "/deps/onnxruntime")

# Search ONNXRUNTIME_ROOT first if it is set.
if(ONNXRUNTIME_ROOT)
  set(_ONNXRUNTIME_SEARCH_ROOT PATHS ${ONNXRUNTIME_ROOT} NO_DEFAULT_PATH)
  list(APPEND _ONNXRUNTIME_SEARCHES _ONNXRUNTIME_SEARCH_ROOT)
endif()

# Find include directory
foreach(search ${_ONNXRUNTIME_SEARCHES})
  find_path(ONNXRUNTIME_INCLUDE_DIR NAMES "onnxruntime_c_api.h" ${${search}} PATH_SUFFIXES include)
endforeach()
mark_as_advanced(ONNXRUNTIME_INCLUDE_DIR)

if(NOT ONNXRUNTIME_LIBRARY)
  # Find all ONNXRuntime libraries
  foreach(search ${_ONNXRUNTIME_SEARCHES})
    find_library(ONNXRUNTIME_LIBRARY_RELEASE NAMES onnxruntime NAMES_PER_DIR ${${search}} PATH_SUFFIXES lib)
    find_library(ONNXRUNTIME_LIBRARY_DEBUG NAMES onnxruntimed NAMES_PER_DIR ${${search}} PATH_SUFFIXES lib)
  endforeach()

  include(SelectLibraryConfigurations)
  select_library_configurations(ONNXRUNTIME)
  mark_as_advanced(ONNXRUNTIME_LIBRARY_RELEASE ONNXRUNTIME_LIBRARY_DEBUG)
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ONNXRuntime
                                  FOUND_VAR ONNXRUNTIME_FOUND
                                  REQUIRED_VARS ONNXRUNTIME_LIBRARY
                                                ONNXRUNTIME_INCLUDE_DIR
                                  FAIL_MESSAGE "Failed to find ONNXRuntime")

if(ONNXRUNTIME_FOUND)
  set(ONNXRUNTIME_INCLUDE_DIRS "${ONNXRUNTIME_INCLUDE_DIR}")
  set(ONNXRUNTIME_LIBRARIES "${ONNXRUNTIME_LIBRARY}")
endif()
