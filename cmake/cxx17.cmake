macro(enable_cxx_17 input_target)

    message(STATUS "got target: ${input_target}")

    set_target_properties(${input_target} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS YES
    )

    target_compile_definitions(${input_target} PRIVATE _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING)
    target_compile_definitions(${input_target} PRIVATE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
    target_compile_definitions(${input_target} PRIVATE _SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING)
    
# target_compile_definitions(${target} )

endmacro(enable_cxx_17)