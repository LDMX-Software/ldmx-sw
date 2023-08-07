# from here:
#
# https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Available.md
# Originally by Jason Turner/lefticus, released under the "unlicense" license
# into the public domain

function(
  enable_compiler_warnings
  project_name
  )
  option(WARNINGS_AS_ERRORS "Build LDMX-sw while treating all warnings as errors" OFF)
  option(PEDANTIC_WARNINGS "Build LDMX-sw with pedantic compiler warnings enabled" OFF)
  option(ADDITIONAL_WARNINGS "Build LDMX-sw with additional compiler warnings enabled" OFF)

  if (ADDITIONAL_WARNINGS)
    if(PEDANTIC_WARNINGS)
      set(PEDANTIC_WARNINGS_ENABLED
        -Wpedantic # warn if non-standard C++ is used
      )
    else()
      set(PEDANTIC_WARNINGS_ENABLED "")
    endif()
    if("${CLANG_WARNINGS}" STREQUAL "")
      set(CLANG_WARNINGS
        -Wall
        -Wextra # reasonable and standard
        -Wshadow # warn the user if a variable declaration shadows one from a parent context
        -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps
        # catch hard to track down memory errors
        -Wcast-align # warn for potential performance problem casts
        -Wunused # warn on anything being unused
        -Woverloaded-virtual # warn if you overload (not override) a virtual function
        -Wnull-dereference # warn if a null dereference is detected
        -Wformat=2 # warn on security issues around functions that format output (ie printf)
        -Wimplicit-fallthrough # warn on statements that fallthrough without an explicit annotation
        ${DISABLED_WARNINGS}
        ${PEDANTIC_WARNINGS_ENABLED}
      )
    endif()

    if("${GCC_WARNINGS}" STREQUAL "")
      set(GCC_WARNINGS
        ${CLANG_WARNINGS}
        -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
        -Wduplicated-cond # warn if if / else chain has duplicated conditions
        -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
        -Wuseless-cast # warn if you perform a cast to the same type
      )
    if("${DISABLED_WARNINGS}" STREQUAL "")
      set(DISABLED_WARNINGS
        # These are used so much in ldmx-sw that we can't warn on them
        -Wno-old-style-cast # warn for c-style casts
        -Wno-unused-parameter
        -Wno-sign-conversion # warn on sign conversions
        -Wno-conversion # warn on type conversions that may lose data
        -Wno-sign-compare
        # Do we care about this one?
        -Wno-double-promotion # warn if float is implicit promoted to double
      )
      list(APPEND CLANG_WARNINGS
        ${DISABLED_WARNINGS}
        # Clang is being standards-compliant and forbidding conversions in
        # {}-initialization. Our code isn't currently standards-compliant here :)
        -Wno-c++11-narrowing
      )
      list(APPEND GCC_WARNINGS
        ${DISABLED_WARNINGS}
        # SimCore sometimes chokes on this one :(
        # -Wduplicated-branches # warn if if / else branches have duplicated code
        -Wno-duplicated-branches
        # Clang will warn if you have private member variables that are never
        # used in the code. You can suppress this with the [[maybe_unused]]
        # attribute. However, GCC does not provide such a warning and therefore
        # thinks that the attribute is meaningless.
        #
        # See
        # https://stackoverflow.com/questions/50646334/maybe-unused-on-member-variable-gcc-warns-incorrectly-that-attribute-is
        -Wno-attributes
      )
    endif()

    endif()

  endif()


  if(WARNINGS_AS_ERRORS)
    list(APPEND CLANG_WARNINGS -Werror)
    list(APPEND GCC_WARNINGS -Werror)
  endif()
  if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(PROJECT_WARNINGS_CXX ${CLANG_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(PROJECT_WARNINGS_CXX ${GCC_WARNINGS})
  else()
    message(AUTHOR_WARNING "No compiler warnings set for CXX compiler: '${CMAKE_CXX_COMPILER_ID}'")
    # TODO support Intel compiler
  endif()

  # use the same warning flags for C
  set(PROJECT_WARNINGS_C "${PROJECT_WARNINGS_CXX}")

  # if (NOT
  #     "${project_name}"
  #     MATCHES
  #     "^SimCore"
  #   )
  target_compile_options(
    ${project_name}
    INTERFACE # C++ warnings
              $<$<COMPILE_LANGUAGE:CXX>:${PROJECT_WARNINGS_CXX}>
              # C warnings
              $<$<COMPILE_LANGUAGE:C>:${PROJECT_WARNINGS_C}>
              )
   # else()
   #   message(FATAL_ERROR "Not doing warnings for ${project_name}")
   # endif()
endfunction()
