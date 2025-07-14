function(set_target_compiler_flags target_name)
  if(MSVC)
    target_compile_options(${target_name} PRIVATE $<$<CONFIG:Release>:/O2>)
    target_compile_options(${target_name} PRIVATE /MP)
  else()
    target_compile_options(${target_name} PRIVATE $<$<CONFIG:Release>:-O2>)
  endif()
endfunction()

