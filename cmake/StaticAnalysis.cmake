
macro(enable_clang_tidy target WARNINGS_AS_ERRORS)


  option(ENABLE_CLANG_TIDY "Enable running clang-tidy with the build system" OFF)

  if(ENABLE_CLANG_TIDY)
    find_program(CLANGTIDY clang-tidy)
    if(CLANGTIDY)
      set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
      # construct the clang-tidy command line
      set(CLANG_TIDY_OPTIONS
        ${CLANGTIDY}
        -extra-arg=-Wno-unknown-warning-option
        -extra-arg=-Wno-ignored-optimization-argument
        -extra-arg=-Wno-unused-command-line-argument
        -p)
      # set standard
      if(NOT
          "${CMAKE_CXX_STANDARD}"
          STREQUAL
          "")
        set(CLANG_TIDY_OPTIONS ${CLANG_TIDY_OPTIONS} -extra-arg=-std=c++${CMAKE_CXX_STANDARD})
      endif()

      # set warnings as errors
      if(${WARNINGS_AS_ERRORS})
        list(APPEND CLANG_TIDY_OPTIONS -warnings-as-errors=*)
      endif()

      set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_OPTIONS})
      set_target_properties(${target} PROPERTIES "${CLANGTIDY}" "${CMAKE_CXX_CLANG_TIDY}")
    else()
      message(${WARNING_MESSAGE} "clang-tidy requested but executable not found")
    endif()
  endif()
endmacro()
