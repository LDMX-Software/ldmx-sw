#Set the minimum version of CMake that's required
cmake_minimum_required(VERSION 3.12)

#Set the project name
project(biasing 
  VERSION 
    2.1.0 
  DESCRIPTION 
    "Biasing module." 
  LANGUAGES 
    C CXX
)

set(biasing_sources
  ${biasing_SOURCE_DIR}/src/Biasing/Utility/StepPrinter.cxx
  ${biasing_SOURCE_DIR}/src/Biasing/TargetBremFilter.cxx
  ${biasing_SOURCE_DIR}/src/Biasing/TargetProcessFilter.cxx
)

add_library(biasing SHARED
  ${biasing_sources}
)

target_link_libraries(biasing PUBLIC 
  Geant4::Interface
  g4fire::g4fire
  fire::framework
)

target_include_directories(biasing PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>"
)

install(TARGETS biasing 
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

# need pattern to exclude version header configured by CMake above
install(DIRECTORY include DESTINATION include FILES_MATCHING PATTERN "*.h")

configure_file(${biasing_SOURCE_DIR}/python/include.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/python/include.py)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/include.py
  DESTINATION ${CMAKE_INSTALL_PREFIX}/python/biasing/)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python/
  DESTINATION ${CMAKE_INSTALL_PREFIX}/python/biasing
  FILES_MATCHING PATTERN "*.py")
