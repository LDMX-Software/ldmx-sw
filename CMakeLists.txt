
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(MagFieldMap VERSION 0.1.0
                    DESCRIPTION "Repository for LDMX magnetic field maps."
)

message(STATUS "Installing all field map files.")

# retrieve list of the fieldmap files
file(GLOB fieldmap_files *.tar.gz)
# pass this information to the install script
configure_file(install.cmake.in ${PROJECT_BINARY_DIR}/install.cmake @ONLY)
install(SCRIPT ${PROJECT_BINARY_DIR}/install.cmake)
