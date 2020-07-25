
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Biasing VERSION 2.1.0
                DESCRIPTION "Biasing classes."
                LANGUAGES CXX
)

# Configure Geant4
setup_geant4_target()

setup_library(name Biasing
              dependencies Geant4::Interface DARK::Framework DARK::SimCore
              python_install_path ${CMAKE_INSTALL_PREFIX}/python/LDMX
)
