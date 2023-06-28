# Taken from https://github.com/cpp-best-practices/cpp_starter_project
# Originally by Jason Turner/lefticus, released under the "unlicense" license
# into the public domain

# Adds build options to turn on the various sanitizers to a target, all turned off by default.
# - ENABLE_SANITIZER_ADDRESS
# - ENABLE_SANITIZER_LEAK
# - ENABLE_SANITIZER_UNDEFINED_BEHAVIOR
# - ENABLE_SANITIZER_THREAD
# - ENABLE_SANITIZER_MEMORY
#
# each of which turns on the corresponding -fsanitize= flag for the project_name
# argument, both as compilation and
#
# Note that some options are incompatible, which will produce warnings.
#
# By default, some sanitizers will stop immediately on spotting an error while
# others will let execution continue. If you want to change this behavior, you
# can add the corresponding SANITIZER_X_RELAXED or SANITIZER_X_STRICT.
#
# These are currently implemented for the address sanitizer (relaxed) and
# undefined behavior (strict). Note that even with the SANITIZER_ADDRESS_RELAXED
# flag enabled, ASAN will still stop on first error unless you also add
# halt_on_error=0 to the ASAN_OPTIONS environment variable. If you are working
# within the container environment, you can do this by running
#
# ldmx setenv ASAN_OPTIONS=halt_on_error=0
function(enable_sanitizers project_name)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")

    set(SANITIZERS "")
    set(SANITIZERS_RECOVERY "")
    set(SANITIZERS_STRICT "")

    option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(SANITIZER_ADDRESS_RELAXED "Optionally allow for recover on error for ASAN if ASAN_OPTIONS environment variable is set to halt_on_error=0")
    if(ENABLE_SANITIZER_ADDRESS)
      list(APPEND SANITIZERS "address")
      if(SANITIZER_ADDRESS_RELAXED)
        list(APPEND SANITIZERS_RECOVERY "address")
      endif()
    endif()

    option(ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    if(ENABLE_SANITIZER_LEAK)
      list(APPEND SANITIZERS "leak")
    endif()

    option(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR "Enable undefined behavior sanitizer" OFF)
    option(SANITIZER_UNDEFINED_BEHAVIOR_STRICT
        "Enable strict enforcement of undefined behavior sanitizers" OFF
    )
    if(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
      list(APPEND SANITIZERS "undefined")
      if (SANITIZER_UNDEFINED_BEHAVIOR_STRICT)
        list(APPEND SANITIZERS_STRICT "undefined")
      endif()
    endif()

    option(ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    if(ENABLE_SANITIZER_THREAD)
      if("address" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS)
        message(WARNING "Thread sanitizer does not work with Address and Leak sanitizer enabled")
      else()
        list(APPEND SANITIZERS "thread")
      endif()
    endif()

    option(ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    if(ENABLE_SANITIZER_MEMORY AND CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
      message(WARNING "Memory sanitizer requires all the code (including libc++) to be MSan-instrumented otherwise it reports false positives")
      if("address" IN_LIST SANITIZERS
         OR "thread" IN_LIST SANITIZERS
         OR "leak" IN_LIST SANITIZERS)
        message(WARNING "Memory sanitizer does not work with Address, Thread or Leak sanitizer enabled")
      else()
        list(APPEND SANITIZERS "memory")
      endif()
    endif()

    list(
      JOIN
      SANITIZERS
      ","
      LIST_OF_SANITIZERS)
    list(
      JOIN
      SANITIZERS_RECOVERY
      ","
      LIST_OF_SANITIZERS_WITH_RECOVERY)
    list(
      JOIN
      SANITIZERS_STRICT
      ","
      LIST_OF_STRICT_SANITIZERS)
  endif()

  if(LIST_OF_SANITIZERS)
    if(NOT
       "${LIST_OF_SANITIZERS}"
       STREQUAL
       "")
      target_compile_options(${project_name} INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
      target_link_options(${project_name} INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
      if (NOT
         "${LIST_OF_SANITIZERS_WITH_RECOVERY}"
         STREQUAL
         "")
         target_compile_options(${project_name} INTERFACE -fsanitize-recover=${LIST_OF_SANITIZERS_WITH_RECOVERY})
         target_link_options(${project_name} INTERFACE -fsanitize-recover=${LIST_OF_SANITIZERS_WITH_RECOVERY})
      endif()
      if (NOT
          "${LIST_OF_STRICT_SANITIZERS}"
          STREQUAL
          "")
        target_compile_options(${project_name} INTERFACE -fno-sanitize-recover=${LIST_OF_STRICT_SANITIZERS})
        target_link_options(${project_name} INTERFACE -fno-sanitize-recover=${LIST_OF_STRICT_SANITIZERS})
      endif()
    endif()
  endif()

endfunction()
