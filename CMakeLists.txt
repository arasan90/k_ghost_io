cmake_minimum_required(VERSION 3.10)

if(K_GHOST_IO_DEV)
    project(k_ghost_io LANGUAGES C VERSION 1.0.0)

    include(${CMAKE_CURRENT_LIST_DIR}/k_ghost_io.cmake)
    include(FetchContent)
        FetchContent_Declare(
            gtest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.16.0
        )

    set(k_ghost_io_sources)
    set(k_ghost_io_public_include_dirs)
    set(k_ghost_io_private_include_dirs)
    set(k_ghost_io_private_linked_libs)
    k_ghost_io_get_sources(k_ghost_io_sources)
    k_ghost_io_get_public_headers(k_ghost_io_public_include_dirs)
    k_ghost_io_get_private_headers(k_ghost_io_private_include_dirs)
    k_ghost_io_get_private_linked_libs(k_ghost_io_private_linked_libs)

    add_library(${PROJECT_NAME} STATIC ${k_ghost_io_sources})
    target_include_directories(${PROJECT_NAME} PUBLIC ${k_ghost_io_public_include_dirs})
    target_include_directories(${PROJECT_NAME} PRIVATE ${k_ghost_io_private_include_dirs})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${k_ghost_io_private_linked_libs})

    SET(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -coverage -fprofile-arcs -ftest-coverage")
    SET(GCC_COVERAGE_LINK_FLAGS "-coverage -lgcov")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -Werror ${GCC_COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

    FetchContent_MakeAvailable(gtest)
    enable_testing()
    include(CTest)

    add_subdirectory(test)

    k_ghost_io_create_mock_library()
    k_ghost_io_create_dep_libraries()
else()
    message(FATAL_ERROR "This cmake file is only for development purposes. Please use the main CMakeLists.txt file for building the library.")

endif()