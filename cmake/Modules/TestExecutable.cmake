###############################################################################
# Build and link test executable with all test sources
# must be run after all modules are built
#
# @author Tom Eichlersmith, University of Minnesota
###############################################################################

# find test sources
file(GLOB test_sources ${CMAKE_CURRENT_SOURCE_DIR}/*/test/*.cxx)
if(MODULE_DEBUG)
  message( "test_sources: '${test_sources}'" )
endif()

if(test_sources)

  # test executable must be compiled with all other test sources
  set(executable_test_source Exception/test/ldmx_test.cxx)
  get_filename_component(executable ${executable_test_source} NAME)
  string(REPLACE ".cxx" "" executable ${executable})
  string(REPLACE "_" "-" executable ${executable})
  if(MODULE_DEBUG)
    message("building test executable: ${executable}")
  endif()
  add_executable(${executable} ${executable_test_source} ${test_sources})

  # include all headers
  foreach(module ${MODULES})
    target_include_directories(${executable} PRIVATE ${${module}_INCLUDE_DIR}) #need all module includes
  endforeach()

  # link all external libraries
  file(GLOB external_libs ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/Use*.cmake)
  foreach(external_lib ${external_libs})
    include(${external_lib}) # include all external libs
  endforeach()
  target_link_libraries(${executable} PUBLIC ${EXT_DEP_LIBRARIES})
  include_directories(${EXT_DEP_INCLUDE_DIRS})
  if(MODULE_DEBUG)
    message("${executable} linked with: ${EXT_DEP_LIBRARIES}")
  endif()

  # link all modules with sources
  foreach(module ${MODULES}) 
    if(EXISTS ${${module}_SOURCE_DIR})
      if(MODULE_DEBUG)
        message("${executable} linked with: ${module}")
      endif()
      target_link_libraries(${executable} PUBLIC ${module})
    endif()
  endforeach()

  # install executable
  install(TARGETS ${executable} DESTINATION bin)

endif()
