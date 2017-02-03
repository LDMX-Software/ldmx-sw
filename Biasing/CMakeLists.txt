# find Geant4
find_package(Geant4 REQUIRED ui_all vis_all)

include_directories("${PROJECT_SOURCE_DIR}/SimApplication/include/")
include_directories("${PROJECT_SOURCE_DIR}/SimPlugins/include")
include_directories("${PROJECT_SOURCE_DIR}/DetDescr/include")


# declare SimPlugins module
module(
  NAME Biasing
  EXTRA_LINK_LIBRARIES ${Geant4_LIBRARIES} 
)

# include Geant4 CMake config file (compiler settings, etc.)
include(${Geant4_USE_FILE})
