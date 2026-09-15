cmake_minimum_required(VERSION 3.12)

# Set the project name
project(DetDescr DESCRIPTION "Detector description module."
                 LANGUAGES CXX
)

setup_python(package_name LDMX/DetDescr)
# Setup the test
setup_test(dependencies DetDescr::DetDescr)

option(BUILD_DETECTORID_BINDINGS "Build the python bindings for the the DetDescr/DetectorID components" ON)
if(BUILD_DETECTORID_BINDINGS)
  cmake_policy(SET CMP0167 NEW)
  find_package(Python COMPONENTS Development)

  message("Looking for libboost_python${Python_VERSION_MAJOR}${Python_VERSION_MINOR}")
  find_package(Boost QUIET COMPONENTS python${Python_VERSION_MAJOR}${Python_VERSION_MINOR})
  # libboost_python3 style libraries
  if (NOT Boost_FOUND)
      message("Looking for libboost_python${Python_VERSION_MAJOR}")
      find_package(Boost QUIET COMPONENTS python${Python_VERSION_MAJOR})
  endif()

  # libboost_python-py36 style libraries
  if (NOT Boost_FOUND)
      message("Looking for libboost_python-py${Python_VERSION_MAJOR}${Python_VERSION_MINOR}")
      find_package(Boost QUIET COMPONENTS python-py${Python_VERSION_MAJOR}${Python_VERSION_MINOR})
  endif()

  # libboost_python3-py36 style libraries
  if (NOT Boost_FOUND)
      message("Looking for libboost_python3-py${Python_VERSION_MAJOR}${Python_VERSION_MINOR}")
      find_package(Boost QUIET COMPONENTS python3-py${Python_VERSION_MAJOR}${Python_VERSION_MINOR})
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
                Python::Python)
  target_compile_definitions(DetDescr PRIVATE DETECTORID_BINDINGS_ENABLED)
else()
  setup_library(module DetDescr
                dependencies  ROOT::Core ROOT::Hist ROOT::Physics
                Framework::Exception Framework::Configure)
endif()
