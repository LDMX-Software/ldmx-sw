
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(EventProc VERSION 2.1.0
                  DESCRIPTION "Module that containts event processors used by LDMX."
                  LANGUAGES CXX
)

setup_library(name EventProc
              dependencies Framework::Framework Event::Event Tools::Tools DetDescr::DetDescr Ecal::Ecal
)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/EventProc)
