
# Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

# Set the project name
project(DQM VERSION 2.1.0
            DESCRIPTION "Data quality management tools."
            LANGUAGES CXX
)

setup_library(name DQM
              dependencies DARK::Framework DARK::Event DARK::Tools DARK::DetDescr)
