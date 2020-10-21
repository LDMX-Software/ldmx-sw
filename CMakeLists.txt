
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Conditions VERSION 2.1.0
                   DESCRIPTION "Module that contains Conditions providers and objects"
                   LANGUAGES CXX
)

setup_library(name Conditions
              dependencies Framework::Framework DetDescr::DetDescr
              python_install_path ${PYTHON_INSTALL_PREFIX}
)

setup_test(dependencies Conditions::Conditions)

setup_python(install_path ${PYTHON_INSTALL_PREFIX}/Conditions)
