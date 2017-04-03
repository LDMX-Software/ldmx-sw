# declare Event module
module(
  NAME Configuration
)

# install configuration data
file(GLOB config_data_items ${CMAKE_CURRENT_SOURCE_DIR}/data/*)
foreach(config_data ${config_data_items})
  install(FILES ${config_data} DESTINATION data/config PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
endforeach()
