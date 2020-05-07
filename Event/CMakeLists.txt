
# Execute the command to extract the SHA1 hash of the current git tag. 
# The variable GIT_SHA1 will be set to contain the hash. 
execute_process (
    COMMAND git rev-parse HEAD
    OUTPUT_VARIABLE GIT_SHA1
)

# Remove the newline character
string(REGEX REPLACE "\n$" "" GIT_SHA1 "${GIT_SHA1}")

# Copies the file 'Version.h.in', substitutes the value of GIT_SHA1 and
# writes it out to Version.h.  
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Event/Version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/Event/Version.h
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/Event/Version.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Event/)

# Add the location of Version.h to the build. Also, set the variable
# EXTRA_INC in the parent scope so the extra include directory can 
# be added to the whole project not just this module.
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include/)
set(EXTRA_INC ${CMAKE_CURRENT_BINARY_DIR}/include/ PARENT_SCOPE)

# Declare Event module
module(
  NAME Event
  EXTRA_SOURCES EventDict.cxx
  EXTERNAL_DEPENDENCIES ROOT
)

# generate ROOT dictionary for Event classes
root_generate_dictionary(${PROJECT_NAME}Dict ${${PROJECT_NAME}_INCLUDE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Def.h LINKDEF ${${PROJECT_NAME}_INCLUDE_DIR}/${PROJECT_NAME}/${PROJECT_NAME}LinkDef.h)

# install ROOT pcm file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}Dict_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
