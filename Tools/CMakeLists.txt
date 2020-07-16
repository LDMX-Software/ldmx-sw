

# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(Tools VERSION 2.1.0
              DESCRIPTION "Module containing helper classes."
              LANGUAGES CXX
)

setup_library(name Tools
              dependencies DARK::Framework DARK::Event)
