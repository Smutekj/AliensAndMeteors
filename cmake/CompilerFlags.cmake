function(set_compiler_flags target_name)
  if(MSVC)
    target_compile_options(${target_name} PRIVATE $<$<CONFIG:Release>:/O2>)
  else()
    target_compile_options(${target_name} PRIVATE $<$<CONFIG:Release>:-O2>)
  endif()

endfunction()

