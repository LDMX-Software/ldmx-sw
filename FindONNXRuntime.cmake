#
# FindONNXRuntime.cmake
#
# Finds the ONNX Runtime binaries or download them if a suitable version isn't
# found.
#
# This will define the following variables
#
# ONNXRuntime_FOUND ONNXRuntime_INCLUDE_DIRS - path to headers
# ONNXRuntime_VERSION      - version number ONNXRuntime_LIBRARIES    - path to
# libraries
#
# and the following targets
#
# ONNXRuntime::Interface
#

if(NOT ONNXRuntime_FIND_VERSION)
  message(FATAL_ERROR "A version number must be specified")
endif()

macro(download_onnxruntime version destination)

  message(STATUS "Downloading ONNX Runtime v${version} ...")

  # Check what operating system is being used.  This is needed to understand
  # what binaries need to be downloaded.  By default, the OS is assumed to be a
  # Linux/Unix variant. Windows is not supported.
  set(os "linux")
  if(APPLE)
    set(os "osx")
  endif()

  # Set the URL from which to download the binaries from
  set(file_prefix "onnxruntime-${os}-x64-${version}")
  set(url
      https://github.com/microsoft/onnxruntime/releases/download/v${version}/${file_prefix}.tgz
  )

  # Download the file
  file(DOWNLOAD ${url} ${destination}/${file_prefix}.tgz STATUS status)

  # If the file fails to download, throw a fatal error and exit
  list(GET status 0 error)
  if(error)
    message(FATAL_ERROR "Could not download ${url}")
  endif()

  # Extract the archive
  execute_process(COMMAND tar -zxvf ${file_prefix}.tgz
                  WORKING_DIRECTORY ${destination})

  # Set the path to the include and libraries to facilitate finding the
  # dependency.
  set(ONNXRuntime_INCLUDE_DIRS
      "${destination}/${file_prefix}/include"
      CACHE INTERNAL "")
  set(ONNXRuntime_LIBRARIES
      "${destination}/${file_prefix}/lib"
      CACHE INTERNAL "")
  set(ONNXRuntime_VERSION
      ${version}
      CACHE INTERNAL "")

endmacro()

if(NOT TARGET ONNXRuntime::Interface)
  find_path(
    onnxruntime_include_dir
    NAMES onnxruntime_cxx_api.h
    PATHS ${ONNXRuntime_INCLUDE_DIRS}
    PATH_SUFFIXES onnxruntime)

  # If onnxruntime isn't found, download it.
  if(NOT onnxruntime_include_dir)
    download_onnxruntime(1.2.0 ${CMAKE_INSTALL_PREFIX}/external)
  endif()

  # Create the target
  add_library(ONNXRuntime::Interface SHARED IMPORTED GLOBAL)
  set_target_properties(
    ONNXRuntime::Interface
    PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ONNXRuntime_INCLUDE_DIRS}"
               IMPORTED_LOCATION "${ONNXRuntime_LIBRARIES}/libonnxruntime.so")

  message(STATUS "Found ONNX Runtime version ${ONNXRuntime_VERSION}")
  set(ONNXRuntime_FOUND TRUE)
endif()
