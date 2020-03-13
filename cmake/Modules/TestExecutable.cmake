# Build and link test executable with all test sources
# must be run after all modules are built

# find test sources
file(GLOB test_sources ${CMAKE_CURRENT_SOURCE_DIR}/*/test/*.cxx)
if(MODULE_DEBUG)
  message( "test_sources: '${test_sources}'" )
endif()

if(test_sources)
  # test executable must be linked to all other module directories
  set(executable_test_source Exception/test/ldmx_test.cxx)
  get_filename_component(executable ${executable_test_source} NAME)
  string(REPLACE ".cxx" "" executable ${executable})
  string(REPLACE "_" "-" executable ${executable})
  if(MODULE_DEBUG)
    message("building test executable: ${executable}")
  endif()
  add_executable(${executable} ${executable_test_source} ${test_sources})
  foreach(module ${MODULES})
      target_include_directories(${executable} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${module}/include) # need all module headers
  endforeach()
  file(GLOB external_libs ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/Use*.cmake)
  foreach(external_lib ${external_libs})
    include(${external_lib}) # include all external libs
  endforeach()
  target_link_libraries(${executable} PUBLIC ${MODULES} ${EXT_DEP_LIBRARIES}) # link all module and external libraries
  install(TARGETS ${executable} DESTINATION bin)
  if(MODULE_DEBUG)
      message("${executable} linked with: ${MODULES};${EXT_DEP_LIBRARIES}")
  endif()
endif()
