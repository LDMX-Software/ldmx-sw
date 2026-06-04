project(Detectors)
add_custom_target(Detectors ALL)
# install mojo to make sure GDML module names are absolute paths pointing to installed files
set(detector_install_path ${CMAKE_INSTALL_PREFIX}/data/detectors)
set(magmap_install_path ${CMAKE_INSTALL_PREFIX}/data/fieldmap)
set(detector_data ${CMAKE_CURRENT_SOURCE_DIR}/data)
file(GLOB detectors RELATIVE ${detector_data} ${detector_data}/*) 
find_package(Python)#No need to get fancy with it

#loop through all the detector versions
foreach(detector ${detectors})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/${detector})
    #this is a directory, assume it is a detector we need to update and install
    set(detector_dir ${CMAKE_CURRENT_SOURCE_DIR}/data/${detector})
    #If directory contains any python scripts, execute those first
    #This is intended for use to generate GDML files that need some extra parameters
    #but can be extended to any additional scripts which need to run before building
    file(GLOB py_scripts RELATIVE ${detector_dir} ${detector_dir}/*.py)
    foreach(py_script ${py_scripts})
      execute_process(#execute_process is always called PRE-build
	COMMAND	${Python_EXECUTABLE} ${detector_dir}/${py_script}
      )
    endforeach()
    #Go through the gdml files here and replace relative paths with full paths
    file(GLOB gdml_files RELATIVE ${detector_dir} ${detector_dir}/*.gdml)
    foreach(gdml_file ${gdml_files})
      set(input ${CMAKE_CURRENT_SOURCE_DIR}/data/${detector}/${gdml_file})
      set(output ${CMAKE_CURRENT_BINARY_DIR}/data/${detector}/${gdml_file})
      add_custom_command(
        OUTPUT ${output}
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/data/${detector}
        COMMAND sed -E "'s|\"([A-Za-z_0-9]*.gdml)\"|\"${detector_install_path}/${detector}/\\1\"|g;s|\"(Bmap.*dat)\"|\"${magmap_install_path}/\\1\"|g;s|\"([A-Za-z_0-9]*.xsd)\"|\"${detector_install_path}/xsd_files/\\1\"|g'" ${input} > ${output}
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
    #Also copy over the XSD files, if any
    file(GLOB xsd_files RELATIVE ${detector_dir} ${detector_dir}/*.xsd)
    foreach(xsd_file ${xsd_files})      
      install(
	FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/${detector}/${xsd_file}
	DESTINATION data/detectors/${detector}
      )
    endforeach()
    
  endif()
endforeach()

#Add visualization directory and XSD files directory to build
install(
  DIRECTORY ${detector_data}/visualization/
  DESTINATION data/detectors/visualization
)
install(
  DIRECTORY ${detector_data}/xsd_files/
  DESTINATION data/detectors/xsd_files
)

# configure python scripts that need cmake variables
setup_python(package_name LDMX/Detectors)
