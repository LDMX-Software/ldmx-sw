#
# FindLDMXSW.cmake
#
# Find a local installation of ldmx-sw. 
# 
# This will define the following variables
#
#   LDMXSW_FOUND
#   LDMXSW_INCLUDE_DIRS - path to the headers
#   LDMXSW_LIBRARIES   - path to the libraries
#
# and the following targets
#
#   LDMXSW::LDMXSW
#
#

if (NOT TARGET LDMXSW::Interface)
  find_path(ldmxsw_include_dir
    NAMES Process.h
    PATHS ${LDMXSW_INSTALL_PREFIX}/include
    PATH_SUFFIXES Framework
  )

  # If ldmx-sw isn't found, throw an error
  if(NOT ldmxsw_include_dir)
    message( FATAL_ERROR "ldmx-sw install not found. Give the path as 
      '-DLDMX_INSTALL_PREFIX=<path-to-ldmx-sw-install>'" )
  endif()

  # Set all variables
  set(LDMXSW_INCLUDE_DIRS ${LDMXSW_INSTALL_PREFIX}/include)
  set(LDMXSW_LIBRARIES ${LDMXSW_INSTALL_PREFIX}/lib)
  file(GLOB LDMXSW_LIBS ${LDMXSW_LIBRARIES}/*.so)

  # Create the target
  add_library(LDMXSW::Interface INTERFACE IMPORTED GLOBAL)
  set_target_properties(LDMXSW::Interface
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${LDMXSW_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${LDMXSW_LIBS}"
  )

  message(STATUS "Found ldmx-sw at: ${LDMXSW_INSTALL_PREFIX}")
  set(LDMXSW_FOUND TRUE)
endif()
