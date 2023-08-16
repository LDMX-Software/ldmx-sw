add_custom_target(Detectors ALL)
# install mojo to make sure GDML module names are absolute paths pointing to installed files
set(detector_install_path ${CMAKE_INSTALL_PREFIX}/data/detectors)
set(magmap_install_path ${CMAKE_INSTALL_PREFIX}/data/fieldmap)
set(detector_data ${CMAKE_CURRENT_SOURCE_DIR}/data)
file(GLOB detectors RELATIVE ${detector_data} ${detector_data}/*) 
#loop through all the detector versions
foreach(detector ${detectors})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/${detector})
    #this is a directory, assume it is a detector we need to update and install
    #  go through the gdml files here and replace relative paths with full paths
    set(detector_dir ${CMAKE_CURRENT_SOURCE_DIR}/data/${detector})
    file(GLOB gdml_files RELATIVE ${detector_dir} ${detector_dir}/*.gdml)
    foreach(gdml_file ${gdml_files})
      set(input ${CMAKE_CURRENT_SOURCE_DIR}/data/${detector}/${gdml_file})
      set(output ${CMAKE_CURRENT_BINARY_DIR}/data/${detector}/${gdml_file})
      add_custom_command(
        OUTPUT ${output}
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/data/${detector}
        COMMAND sed -E "'s|\"([A-Za-z_0-9]*.gdml)\"|\"${detector_install_path}/${detector}/\\1\"|g;s|\"(Bmap.*dat)\"|\"${magmap_install_path}/\\1\"|g'" ${input} > ${output}
        DEPENDS ${input}
        COMMENT "Writing full paths to ${detector}/${gdml_file}"
      )
      add_custom_target(${detector}_${gdml_file} DEPENDS ${output})
      add_dependencies(Detectors ${detector}_${gdml_file})
      install(
        FILES ${output}
        DESTINATION data/detectors/${detector}
      )
    endforeach()
  endif()
endforeach()

# configure python scripts that need cmake variables
file(GLOB config_scripts ${CMAKE_CURRENT_SOURCE_DIR}/python/*.in)
foreach(config_script ${config_scripts})
  string(REPLACE ".in" "" config_script_output ${config_script})
  get_filename_component(config_script_output ${config_script_output} NAME)
  configure_file(${config_script} ${CMAKE_CURRENT_BINARY_DIR}/python/${config_script_output})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/${config_script_output} 
         DESTINATION ${CMAKE_INSTALL_PREFIX}/python/LDMX/Detectors)
endforeach()
