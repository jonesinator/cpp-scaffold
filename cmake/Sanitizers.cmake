if(ENABLE_ASAN AND ENABLE_TSAN)
    message(FATAL_ERROR "ENABLE_ASAN and ENABLE_TSAN cannot be used together")
endif()

if(ENABLE_ASAN)
    message(STATUS "AddressSanitizer + UndefinedBehaviorSanitizer enabled")
    add_compile_options(
        -fsanitize=address,undefined
        -fno-sanitize-recover=all
        -fno-omit-frame-pointer
    )
    add_link_options(-fsanitize=address,undefined)
endif()

if(ENABLE_TSAN)
    message(STATUS "ThreadSanitizer enabled")
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()
