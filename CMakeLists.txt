
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Hcal VERSION 2.1.0
             DESCRIPTION "Module for all things HCal."
             LANGUAGES CXX
)


# Search and configure ROOT
find_package(ROOT CONFIG REQUIRED)

setup_library(name Hcal
              dependencies ROOT::Physics 
              Framework::Framework Event::Event Tools::Tools DetDescr::DetDescr 
              python_install_path ${PYTHON_INSTALL_PREFIX}
)

setup_test(dependencies DARK::Hcal)
