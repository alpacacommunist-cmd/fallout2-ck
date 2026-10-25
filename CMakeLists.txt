cmake_minimum_required(VERSION 3.17)
project(fallout2-ck C CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Dev" CACHE STRING "Choose the type of build." FORCE)
endif()

# cmake -B build-dev -DCMAKE_BUILD_TYPE=Dev
if(CMAKE_BUILD_TYPE STREQUAL "Dev")
    set(CMAKE_C_FLAGS_DEV "${CMAKE_C_FLAGS_RELWITHDEBINFO}" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEV "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_DEV "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}" CACHE STRING "" FORCE)

    # ENABLE DEBUG
    string(REPLACE "-DNDEBUG" "" CMAKE_C_FLAGS_DEV "${CMAKE_C_FLAGS_DEV}")
    string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_DEV "${CMAKE_CXX_FLAGS_DEV}")
    # msvc
    string(REPLACE "/DNDEBUG" "" CMAKE_C_FLAGS_DEV "${CMAKE_C_FLAGS_DEV}")
    string(REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_DEV "${CMAKE_CXX_FLAGS_DEV}")
endif()

# Release-only IPO
# cmake -B build-release -DCMAKE_BUILD_TYPE=Release
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT lto_supported OUTPUT error)
    if(lto_supported)
        message(STATUS "LTO/IPO enabled!")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    endif()
else()
    message(STATUS "(${CMAKE_BUILD_TYPE}). LTO disabled.")
endif()

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

            target_link_options(${TARGET} PRIVATE $<$<CONFIG:Release,MinSizeRel>:-s>)
            target_link_options(${TARGET} PRIVATE $<$<CONFIG:Dev,MinSizeRel>:-s>)
        endif()
    endif()
endforeach()
