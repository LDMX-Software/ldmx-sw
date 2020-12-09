
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Recon VERSION 2.1.0
              DESCRIPTION "Module containing recon processors that span multiple
                           systems."
              LANGUAGES CXX
)

file(GLOB SRC_FILES CONFIGURE_DEPENDS 
  ${PROJECT_SOURCE_DIR}/src/Recon/Examples/*.cxx
  ${PROJECT_SOURCE_DIR}/src/Recon/Overlay/*.cxx
  ${PROJECT_SOURCE_DIR}/src/Recon/Skims/*.cxx
  ${PROJECT_SOURCE_DIR}/src/Recon/Trigger/*.cxx
)

setup_library(name Recon
              dependencies Framework::Framework Event::Event Tools::Tools 
                           DetDescr::DetDescr Ecal::Ecal
              sources ${SRC_FILES}
)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/Recon)
