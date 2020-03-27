# declare Event module
module(
  NAME Configuration
)

# install configuration data
file(GLOB config_data_items ${CMAKE_CURRENT_SOURCE_DIR}/data/*)
foreach(config_data ${config_data_items})
  install(FILES ${config_data} DESTINATION data/config PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
endforeach()

# configure python scripts that need cmake variables
file(GLOB config_scripts ${CMAKE_CURRENT_SOURCE_DIR}/python/*.in)
foreach(config_script ${config_scripts})
  string(REPLACE ".in" "" config_script_output ${config_script})
  get_filename_component(config_script_output ${config_script_output} NAME)
  configure_file(${config_script} ${CMAKE_CURRENT_BINARY_DIR}/python/${config_script_output})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/${config_script_output} DESTINATION lib/python/LDMX/Configuration)
endforeach()
