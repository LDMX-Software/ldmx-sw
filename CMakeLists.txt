
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(MagFieldMap VERSION 0.1.0
                    DESCRIPTION "Repository for LDMX magnetic field maps."
)

message(STATUS "Installing all field map files.")

# Retrieve all of the fieldmap files
file(GLOB fieldmap_files *.tar.gz)

# Check if the fieldmap has already been extracted.  If not, extract and install
# the fieldmaps.
install(CODE "execute_process(COMMAND mkdir -p ${CMAKE_INSTALL_PREFIX}/data/fieldmap)")
foreach(fieldmap_path ${fieldmap_files})
  get_filename_component(fieldmap_file ${fieldmap_path} NAME)
  install(CODE "message(STATUS \"unpacking ${fieldmap_file}\")")
  install(CODE "execute_process(COMMAND tar -zxf ${fieldmap_path} WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/data/fieldmap)")
endforeach()
