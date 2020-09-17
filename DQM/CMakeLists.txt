
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(DQM VERSION 2.1.0
            DESCRIPTION "Data quality management tools."
            LANGUAGES CXX
)

# Search and configure ROOT
find_package(ROOT CONFIG REQUIRED)

setup_library(name DQM
              dependencies ROOT::Physics 
                           Framework::Framework Event::Event Tools::Tools DetDescr::DetDescr
              python_install_path ${PYTHON_INSTALL_PREFIX}
)
