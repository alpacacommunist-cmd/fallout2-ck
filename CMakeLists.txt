cmake_minimum_required(VERSION 3.17)
project(fallout2-ck C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# includes alias
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(FalloutCeAlias)
setup_fallout2_ce_alias()

# submodule
set(FALLOUT_VENDORED ON CACHE BOOL "" FORCE)
add_subdirectory(fallout2-ce)

# vendor
add_subdirectory(src/vendor)

# fallout2-ck
add_subdirectory(src)

# fallout2-ce
foreach(TARGET fallout2-ce mapper-ce)
    if(TARGET ${TARGET})
        target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
        target_link_libraries(${TARGET} fallout2-ck)
        set_target_properties(${TARGET} PROPERTIES ENABLE_EXPORTS TRUE)

        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer>)
            target_link_options(${TARGET} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
        endif()
    endif()
endforeach()
