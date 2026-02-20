if (BUILD_DEMOS)

    add_subdirectory(basic_demo)
    add_subdirectory(heat_transfer)
    add_subdirectory(elastic_demo)

    if (BUILD_DYN_PROC_DEMOS)
        add_subdirectory(dyn_proc_demo)
    endif ()

    if (PROFILING)
        add_subdirectory(profiling)
    endif ()

    if (TEST_SCALAPACK)
        add_subdirectory(scalapack_demo)
    endif ()

endif ()