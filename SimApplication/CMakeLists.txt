# project name
project(SimApplication)

# find Geant4
find_package(Geant4 REQUIRED ui_all vis_all)

# include Geant4 CMake config file (compiler settings, etc.)
include(${Geant4_USE_FILE})

# find ROOT
find_package(ROOT REQUIRED)

# add project include dir
include_directories(${PROJECT_SOURCE_DIR}/include)

# add Event module include dir
include_directories(${Event_INCLUDE_DIR})

# add ROOT include dirs
include_directories(${ROOT_INCLUDE_DIRS})

# list sources for building the application
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h)

# set the output executable
add_executable(ldmx-sim src/ldmx_sim.cc ${sources} ${headers})

# add the Geant4 link libraries
target_link_libraries(ldmx-sim ${Geant4_LIBRARIES} Event)

# set executable installation target
install(TARGETS ldmx-sim DESTINATION bin)
