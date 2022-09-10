
# install_gdml(detector_name)
#
#   Install a directory of GDML files, updating any relative references
#   to their full path to the install location.
#
# ARGUMENTS:
#  detector_name - name of subdirectory holding detector version
function(install_gdml)
  cmake_parse_arguments(install_gdml "" "detector_name" "" ${ARGN})

  set(detector_dir ${CMAKE_CURRENT_SOURCE_DIR}/data/${install_gdml_detector_name})
  file(GLOB gdml_files RELATIVE ${detector_dir} ${detector_dir}/*.gdml)
  foreach(gdml_file ${gdml_files})
    # read in GDML file contents
    file(READ ${detector_dir}/${gdml_file} file_contents)
    # get matches for other GDML files
    string(REGEX MATCHALL \"[A-Za-z_0-9]*.gdml\" gdml_matches "${file_contents}")
    foreach(gdml_match ${gdml_matches})
      # replace relative paths with absolute path
      string(REPLACE "\"" "" gdml_name ${gdml_match})
      set(gdml_full_path 
        ${CMAKE_INSTALL_PREFIX}/data/detectors/${install_gdml_detector_name}/${gdml_name})
      string(REPLACE ${gdml_name} ${gdml_full_path} file_contents "${file_contents}")
    endforeach()
    # get match(es) to the magnetic field map
    string(REGEX MATCHALL \"Bmap.*dat\" mag_map_matches "${file_contents}")
    foreach (mag_map_match ${mag_map_matches})
      #replace relative paths to absolute install paths for these mag field maps
      string(REPLACE "\"" "" mag_map_name ${mag_map_match})
      set(mag_map_path ${CMAKE_INSTALL_PREFIX}/data/fieldmap/${mag_map_name})
      string(REPLACE ${mag_map_name} ${mag_map_path} file_contents "${file_contents}")
    endforeach()
    # done searching and replacing --> write out file and install the new copy
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/data/${install_gdml_detector_name}/${gdml_file}
              "${file_contents}")
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/data/${install_gdml_detector_name}/${gdml_file}
            DESTINATION data/detectors/${install_gdml_detector_name}
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ)
  endforeach()
endfunction()

# install mojo to make sure GDML module names are absolute paths pointing to installed files
set(detector_data ${CMAKE_CURRENT_SOURCE_DIR}/data)
file(GLOB detectors RELATIVE ${detector_data} ${detector_data}/*) 
#loop through all the detector versions
foreach(detector ${detectors})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/${detector})
    #this is a directory
    #   go through detector.gdml and replace the relative paths with aboslute paths to the installation
    install_gdml(detector_name ${detector})
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
