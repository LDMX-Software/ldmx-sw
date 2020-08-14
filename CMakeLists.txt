
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(MagFieldMap VERSION 2.1.0
                    DESCRIPTION "Repository for LDMX magnetic field maps."
)

message(STATUS "Installing all field map files.")

# Retrieve all of the fieldmap files
file(GLOB fieldmap_files *.tar.gz)

# Check if the fieldmap has already been extracted.  If not, extract and install
# the fieldmaps.
foreach(fieldmap_path ${fieldmap_files})
    get_filename_component(fieldmap_file ${fieldmap_path} NAME)
    string(REPLACE ".tar.gz" "" fieldmap_file_raw ${fieldmap_file})
    if (NOT EXISTS ${CMAKE_INSTALL_PREFIX}/data/fieldmap/${fieldmap_file_raw})
        configure_file(${fieldmap_path} ${CMAKE_INSTALL_PREFIX}/data/fieldmap/${fieldmap_file} COPYONLY)
        execute_process(COMMAND tar -zxvf ${fieldmap_file}
                        WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/data/fieldmap
        )
        file(REMOVE ${CMAKE_INSTALL_PREFIX}/data/fieldmap/${fieldmap_file})
    endif()
endforeach()
