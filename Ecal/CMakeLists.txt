
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(ECal VERSION 2.1.0
             DESCRIPTION "Module for all things ECal."
             LANGUAGES CXX
)

setup_library(name Ecal
              dependencies DARK::Framework DARK::Event DARK::Tools DARK::DetDescr)


