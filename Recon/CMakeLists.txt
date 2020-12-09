
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Recon VERSION 2.1.0
              DESCRIPTION "Module containing recon processors that span multiple
                           systems."
              LANGUAGES CXX
)

setup_library(name Recon
              dependencies Framework::Framework Event::Event Tools::Tools 
                           DetDescr::DetDescr Ecal::Ecal
)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/Recon)
