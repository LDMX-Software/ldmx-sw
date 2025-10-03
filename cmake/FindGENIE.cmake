#
# FindGENIE.cmake
#
# Find an installation of GENIE. Highly tuned to the installation
# of GENIE that is built into the ldmx/dev image.
#
# This will define the following variables
#
# GENIE_FOUND
# GENIE_INCLUDE_DIRS - path to the headers
# GENIE_LIBRARIES    - CMake list of CMake targets
#
# and the following main target to link to
# (other targets are defined for specific GENIE libraries)
#
# GENIE::GENIE
#

if (GENIE_FOUND)
  return()
endif()

set(GENIE_INCLUDE_DIRS "/usr/local/include/GENIE")
set(GENIE_LIBRARIES "")

macro(add_genie_target)
  cmake_parse_arguments(add_genie_target "" "name" "dependencies" ${ARGN})
  add_library(GENIE::${add_genie_target_name} SHARED IMPORTED)
  set_property(TARGET GENIE::${add_genie_target_name} APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
  set_target_properties(GENIE::${add_genie_target_name} PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "/usr/local/include/GENIE"
    IMPORTED_LOCATION "/usr/local/lib/lib${add_genie_target_name}.so"
    IMPORTED_LOCATION_NOCONFIG "/usr/local/lib/lib${add_genie_target_name}.so"
    IMPORTED_SONAME_NOCONFIG "lib${add_genie_target_name}.so"
  )
  list(LENGTH add_genie_target_dependencies num_deps)
  if (${num_deps} GREATER 0)
    set_target_properties(GENIE::${add_genie_target_name} PROPERTIES
      INTERFACE_LINK_LIBRARIES "${add_genie_target_dependencies}"
    )
  endif()
  list(APPEND GENIE_LIBRARIES GENIE::${add_genie_target_name})
endmacro()

include(CMakeFindDependencyMacro)
find_dependency(ROOT CONFIG REQUIRED)

add_genie_target(name GRwFwk)
add_genie_target(name GRwIO)
add_genie_target(name GRwClc)

add_genie_target(name GFwMsg dependencies log4cpp)
add_genie_target(name GFwReg dependencies GENIE::GFwMsg)
add_genie_target(name GFwAlg dependencies GENIE::GFwReg)
add_genie_target(name GFwInt dependencies GENIE::GFwAlg)
add_genie_target(name GFwGHEP)
add_genie_target(name GFwNum dependencies ROOT::MathMore ROOT::TreePlayer)
add_genie_target(name GFwUtl dependencies GENIE::GFwInt GENIE::GFwNum pythia8 ROOT::EGPythia8 xml2)
add_genie_target(name GFwParDat dependencies ROOT::EG)
add_genie_target(name GFwEG dependencies GENIE::GFwParDat ROOT::EG)
add_genie_target(name GFwNtp)

add_genie_target(name GPhCmn)
add_genie_target(name GPhDcy)
add_genie_target(name GPhHadTens)
add_genie_target(name GPhMEL)
add_genie_target(name GPhPDF dependencies LHAPDF)
add_genie_target(name GPhXSIg)
add_genie_target(name GPhNuclSt)
add_genie_target(name GPhDeEx)
add_genie_target(name GPhHadnz)
add_genie_target(name GPhHadTransp)
add_genie_target(name GPhAMNGXS)
add_genie_target(name GPhAMNGEG)
add_genie_target(name GPhChmXS)
add_genie_target(name GPhCohXS)
add_genie_target(name GPhCohEG)
add_genie_target(name GPhDISXS)
add_genie_target(name GPhDISEG)
add_genie_target(name GPhDfrcXS)
add_genie_target(name GPhDfrcEG)
add_genie_target(name GPhHELptnXS)
add_genie_target(name GPhHELptnEG)
add_genie_target(name GPhIBDXS)
add_genie_target(name GPhIBDEG)
add_genie_target(name GPhMNucXS)
add_genie_target(name GPhMNucEG)
add_genie_target(name GPhNuElXS)
add_genie_target(name GPhNuElEG)
add_genie_target(name GPhQELXS)
add_genie_target(name GPhQELEG)
add_genie_target(name GPhResXS)
add_genie_target(name GPhResEG)
add_genie_target(name GPhStrXS)
add_genie_target(name GPhStrEG)
add_genie_target(name GPhHEDISXS)
add_genie_target(name GPhHEDISEG)
add_genie_target(name GTlFlx)
add_genie_target(name GTlGeo dependencies ROOT::Geom)

add_library(GENIE::GENIE INTERFACE IMPORTED GLOBAL)
set_target_properties(GENIE::GENIE PROPERTIES
  INTERFACE_LINK_LIBRARIES "${GENIE_LIBRARIES}"
  INTERFACE_INCLUDE_DIRECTORIES "${GENIE_INCLUDE_DIRS}"
)
message(STATUS "Found GENIE")
set(GENIE_FOUND TRUE)
