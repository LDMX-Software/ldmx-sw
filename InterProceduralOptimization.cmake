# Based on https://github.com/cpp-best-practices/cmake_template/blob/main/cmake/InterproceduralOptimization.cmake
# Originally by Jason Turner/lefticus, released under the "unlicense" license
# into the public domain
function(enable_ipo library_name)
  option(ENABLE_LTO "Enable interprocedural/link-time optimization. Drastically slows down link-time if enabled." OFF)
  # if (NOT "${library_name}"
  #     MATCHES
  #     "^SimCore"
  # )

  if (ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result OUTPUT output)
    if(result)
      set_property(TARGET ${library_name} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
      message(SEND_ERROR "IPO is not supported: ${output}")
    endif()
  endif()
  # else()
  #   message(FATAL_ERROR "Not enabling IPO for ${library_name}")
  # endif()
endfunction()
