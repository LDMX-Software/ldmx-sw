# project name
project(SimApplication)

# find Geant4
find_package(Geant4 REQUIRED ui_all vis_all)

# include Geant4 CMake config file (compiler settings, etc.)
include(${Geant4_USE_FILE})

# find ROOT
find_package(ROOT REQUIRED COMPONENTS Core RIO)

# add project include dir
include_directories(${PROJECT_SOURCE_DIR}/include)

# add Event module include dir
include_directories(${Event_INCLUDE_DIR})

# add DetDescr module include dir
include_directories(${DetDescr_INCLUDE_DIR})

# add ROOT include dirs
include_directories(${ROOT_INCLUDE_DIRS})

# list sources for building the application
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cxx)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h)

# set the output executable
add_executable(ldmx-sim src/ldmx_sim.cxx ${sources} ${headers})

# add the Geant4 link libraries and module dep libs
target_link_libraries(ldmx-sim ${Geant4_LIBRARIES} Event DetDescr)

# set executable installation target
install(TARGETS ldmx-sim DESTINATION bin)

# test program
add_executable(ldmx-sim-analysis-test test/ldmx_sim_analysis_test.cxx)
target_link_libraries(ldmx-sim-analysis-test ${ROOT_LIBRARIES} Event DetDescr)
install(TARGETS ldmx-sim-analysis-test DESTINATION bin)
