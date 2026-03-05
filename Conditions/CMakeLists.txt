cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Conditions DESCRIPTION "Module that contains Conditions providers and objects"
                   LANGUAGES CXX
)

find_package(CURL REQUIRED)

setup_library(module Conditions
              dependencies Framework::Framework DetDescr::DetDescr CURL::libcurl
)

setup_test(dependencies Conditions::Conditions)

setup_python(package_name LDMX/Conditions)
