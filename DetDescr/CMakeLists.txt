# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(DetDescr VERSION 2.1.0
                 DESCRIPTION "Detector description module."
                 LANGUAGES CXX
)

# Search and configure ROOT
find_package(ROOT 6.16 CONFIG REQUIRED)

setup_python(package_name ${PYTHON_PACKAGE_NAME}/DetDescr)
# Setup the test
setup_test(dependencies DetDescr::DetDescr)

option(BUILD_DETECTORID_BINDINGS "Build the python bindings for the the DetDescr/DetectorID components" OFF)
find_package(Python COMPONENTS Development)

message("Looking for libboost_python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}")
find_package(Boost QUIET COMPONENTS python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR})
# libboost_python3 style libraries
if (NOT Boost_FOUND)
    message("Looking for libboost_python${PYTHON_VERSION_MAJOR}")
    find_package(Boost QUIET COMPONENTS python${PYTHON_VERSION_MAJOR})
endif()

# libboost_python style libraries
if (NOT Boost_FOUND)
    message("Looking for libboost_python")
    find_package(Boost QUIET COMPONENTS system thread python)
endif()

# libboost_python-py36 style libraries
if (NOT Boost_FOUND)
    message("Looking for libboost_python-py${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}")
    find_package(Boost QUIET COMPONENTS python-py${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR})
endif()

# libboost_python3-py36 style libraries
if (NOT Boost_FOUND)
    message("Looking for libboost_python3-py${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}")
    find_package(Boost QUIET COMPONENTS python3-py${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR})
endif()

# Nothing worked
if (NOT Boost_FOUND)
    message("")
    message(FATAL_ERROR "Failed to find boost libraries!")
endif()


setup_library(module DetDescr
              dependencies  ROOT::Core ROOT::Hist ROOT::Physics
              Framework::Exception Framework::Configure
              Boost::python
              Python::Python
)

