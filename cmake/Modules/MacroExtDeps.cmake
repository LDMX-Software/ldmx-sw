###############################################################################
# Declare external dependencies for a module
#   simply runs the Use file for any external modules listed
#
# @author Tom Eichlersmith, University of Minnesota
###############################################################################

macro(ext_deps)

  # define options for this function
  set(options)
  set(multiValueArgs DEPENDENCIES)
  
  # parse command options
  cmake_parse_arguments(EXT_DEPS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  foreach(ext_dep ${EXT_DEPS_DEPENDENCIES})
    if(MODULE_DEBUG)
      message("including Use${ext_dep}.cmake")
    endif()
    include("Use${ext_dep}")
  endforeach()

endmacro(ext_deps)
