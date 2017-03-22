macro(ext_deps)

  # define options for this function
  set(options)
  set(multiValueArgs DEPENDENCIES)
  
  # parse command options
  cmake_parse_arguments(EXT_DEPS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  #message("multiValueArgs='${multiValueArgs}'")
  #message("EXT_DEPS_DEPENDENCIES='${EXT_DEPS_DEPENDENCIES}'")

  foreach(ext_dep ${EXT_DEPS_DEPENDENCIES})
    #message(${ext_dep})
    #message("including Use${ext_dep}.cmake")
    include("Use${ext_dep}")
  endforeach()

endmacro(ext_deps)
