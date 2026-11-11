cmake_minimum_required(VERSION 3.17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(fallout2-ck C CXX)

option(CK_ENABLE_EDITOR "Build with modder ui/db enabled" ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Dev" CACHE STRING "Choose the type of build." FORCE)
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Dev" "Debug" "Release" "MinSizeRel")

if(CK_ENABLE_EDITOR)
    add_compile_definitions(CK_EDITOR_BUILD)
    message(STATUS "==================================================")
    message(STATUS "  [SDK BUILD] Configuration: ${CMAKE_BUILD_TYPE}")
    message(STATUS "  -> Editor UI (Dear ImGui): ENABLED")
    message(STATUS "  -> Asset Cache (SQLite3): ENABLED")
    message(STATUS "==================================================")
else()
    message(STATUS "==================================================")
    message(STATUS "  [RUNTIME BUILD] Configuration: ${CMAKE_BUILD_TYPE}")
    message(STATUS "  -> Editor UI & Cache: DISABLED (Stripped Release)")
    message(STATUS "==================================================")
endif()

# cmake -B build-dev -DCMAKE_BUILD_TYPE=Dev -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
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

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_link_options("-s")
    endif()
else()
    message(STATUS "(${CMAKE_BUILD_TYPE}). LTO disabled.")
endif()

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
        target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

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
