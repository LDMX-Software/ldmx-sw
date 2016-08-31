# minimum cmake version
cmake_minimum_required(VERSION 3.6)

# project name
project(SimApplication)

# define path to extra CMake modules
set(LDMX_CMAKE_MODULES_ROOT ${PROJECT_SOURCE_DIR}/../cmake/Modules/)

# add dir with extra CMake modules
list(APPEND CMAKE_MODULE_PATH ${LDMX_CMAKE_MODULES_ROOT})

include(MacroCheckPackageVersion)

# find Geant4
find_package(Geant4 REQUIRED)

# include Geant4 CMake config file (compiler settings, etc.)
include(${Geant4_USE_FILE})

# find LDMX EventModel package for ROOT IO
find_package(EventModel REQUIRED)

# add project include dir
include_directories(${PROJECT_SOURCE_DIR}/include)

# list sources for building the application
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h)

# set the output executable
add_executable(ldmx-sim src/ldmx_sim.cc ${sources} ${headers})

# add the Geant4 link libraries
target_link_libraries(ldmx-sim ${Geant4_LIBRARIES})

# set executable installation target
install(TARGETS ldmx-sim DESTINATION bin)
