
# Put the modules in ldmx-sw into a list in dependency order
# the modules list in in cmake variable MODULES

# get list module names 
# a directory in the CMAKE_CURRENT_SOURCE_DIR is considered a module if
#  something named "include", "src", and "CMakeLists.txt" all exists within it
set( UNSORTED "" )
file( GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/* )
foreach( child ${children} )
  if ( (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${child}/include")
   AND (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${child}/src")
   AND (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${child}/CMakeLists.txt" ) )
    #child is correct format to be a module
    list( APPEND UNSORTED ${child} )
  endif()
endforeach()

# Sort modules so that each item has its dependencies listed before it.
# A modules internal dependencies are parsed from its CMakeLists.txt file.
# This will turn in to an infinite loop if there are ANY circular dependencies
#  listed in the CMakeLists.txt files (even if they wouldn't cause a
#  compilation error). Infinite loops are prevented by a FATAL_ERROR.
set( MODULES "" )
list( LENGTH UNSORTED nleft )
while( nleft GREATER 0 )
  #loop until there are no modules left in UNSORTED
  set( addedmodule FALSE )
  foreach( module ${UNSORTED} )
    # get dependency list
    file( STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/${module}/CMakeLists.txt" deps REGEX ".* DEPENDENCIES.*" )
    
    # check if dependencies (if they exist) have already been listed
    set( nodeps TRUE )
    if( NOT( deps STREQUAL "") )
      string( REPLACE " " ";" deps ${deps} )
      #look for dependencies that haven't been sorted yet
      foreach( dep ${deps} )
        list( FIND UNSORTED ${dep} index )
        if( index GREATER -1 )
          #module has dependency that hasn't been listed before it
          set( nodeps FALSE )
          break()
        endif()
      endforeach()
    endif() 
    
    #if module had no dependencies already listed
    if( nodeps )
      list( APPEND MODULES ${module} )
      list( REMOVE_ITEM UNSORTED ${module} )
      set( addedmodule TRUE )
    endif()

  endforeach() #loop through modules in UNSORTED

  list( LENGTH UNSORTED nleft ) #reset number of UNSORTED left to sort

  # infinite loop prevention
  #  if nothing was added on this loop, then nothing will be added on next
  #  loop because conditions didn't change
  if ( NOT(addedmodule) )
    message( FATAL_ERROR "Infinite loop when creating module list. Check for circular dependencies in module CMakeLists.txt" )
  endif()

endwhile() #loop until no modules left in UNSORTED
