cmake_minimum_required(VERSION 3.10)

set(sources
    ${CMAKE_CURRENT_LIST_DIR}/src/${TARGET_PLATFORM}/k_ghost_io.c
)

set(public_includes
    ${CMAKE_CURRENT_LIST_DIR}/include
)

set(private_includes
    ${CMAKE_CURRENT_LIST_DIR}/src
)

set(public_linked_libs
    k_cjson
)

set(private_linked_libs
)


function(k_ghost_io_get_sources OUT_VAR)
    set(${OUT_VAR}
        ${sources}
        PARENT_SCOPE)
endfunction()

function(k_ghost_io_get_public_headers OUT_VAR)
    set(${OUT_VAR}
        ${public_includes}
        PARENT_SCOPE)
endfunction()

function(k_ghost_io_get_private_headers OUT_VAR)
    set(${OUT_VAR}
        ${private_includes}
        PARENT_SCOPE)
endfunction()

function(k_ghost_io_get_private_linked_libs OUT_VAR)
    set(${OUT_VAR}
        ${private_linked_libs}
        PARENT_SCOPE)
endfunction()

function(k_ghost_io_get_public_linked_libs OUT_VAR)
    set(${OUT_VAR}
        ${public_linked_libs}
        PARENT_SCOPE)
endfunction()

function(k_ghost_io_create_mock_library)
    add_library(k_ghost_io_mock ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/mock/k_ghost_io_mock.c)
    target_include_directories(k_ghost_io_mock
        PUBLIC
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/include
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/mock
    )
    target_link_libraries(k_ghost_io_mock PUBLIC k_cjson)
endfunction()

function(k_ghost_io_create_dep_libraries)
    add_subdirectory(${CMAKE_CURRENT_FUNCTION_LIST_DIR}/libs/cjson)
endfunction()