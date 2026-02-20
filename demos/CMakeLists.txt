if (RESHUFFLE_BUILD_DEMOS)

    add_subdirectory(basic_demo)
    add_subdirectory(heat_transfer)
    add_subdirectory(elastic_demo)

    if (RESHUFFLE_BUILD_DYN_PROC_DEMO)
        add_subdirectory(dyn_proc_demo)
    endif ()

    if (RESHUFFLE_PROFILING)
        add_subdirectory(profiling)
    endif ()

    if (RESHUFFLE_TEST_SCALAPACK)
        add_subdirectory(scalapack_demo)
    endif ()

endif ()