#
# FindGENIE.cmake
#
# Find an installation of GENIE. Highly tuned to the installation
# of GENIE that is built into the ldmx/dev image.
#
# This will define the following variables
#
# GENIE_FOUND
# GENIE_INCLUDE_DIRS - path to the headers
# GENIE_LIBRARIES    - CMake list of CMake targets
#
# and the following main target to link to
# (other targets are defined for specific GENIE libraries)
#
# GENIE::GENIE
#

if (GENIE_FOUND)
  return()
endif()

get_filename_component(_thisdir "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${_thisdir}/FindGENIE-target.cmake)
