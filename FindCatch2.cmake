#
# FindCatch2.cmake
#
# Finds the Catch2 header or downloads it if a suitable version isn't found.
#
# This will define the following variables
#
# CATCH2_FOUND CATCH2_INCLUDE_DIR - path to the header catch.hpp CATCH2_VERSION
#
# and the following targets
#
# Catch2::Interface
#

if(NOT Catch2_FIND_VERSION)
  message(FATAL_ERROR "A version number must be specified")
endif()

macro(download_catch2 version destination)

  message(STATUS "Downloading Catch2 v${version} ...")

  # Set the URL from which to download the header from
  set(url
      https://github.com/catchorg/Catch2/releases/download/v${version}/catch.hpp
  )

  # Download the file
  file(DOWNLOAD ${url} ${destination}/catch/catch.hpp STATUS status)

  # If the file fails to download, throw a fatal error and exit
  list(GET status 0 error)
  if(error)
    message(FATAL_ERROR "Could not download ${url}")
  endif()

  # Set the path to the include directory
  set(CATCH2_INCLUDE_DIR
      "${destination}/catch"
      CACHE INTERNAL "")
  set(CATCH2_VERSION
      "${version}"
      CACHE INTERNAL "")

endmacro()

# Search for catch locally
find_path(
  catch2_include_dir
  NAMES catch.hpp
  PATHS ${CATCH2_INCLUDE_DIR}
  PATH_SUFFIXES catch)

message(STATUS "${catch2_include_dir}")

# If Catch2 isn't found, download it.
if(NOT catch2_include_dir)
  download_catch2(${Catch2_FIND_VERSION} ${CMAKE_INSTALL_PREFIX}/external)
endif()

# Create the target
add_library(Catch2::Interface INTERFACE IMPORTED)
set_target_properties(Catch2::Interface PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                                   "${CATCH_INCLUDE_DIR}")

message(STATUS "Found Catch2 version ${CATCH2_VERSION}")

set(CATCH2_FOUND TRUE)
