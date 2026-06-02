# fallout2-ck/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(fallout2-ck CXX)

# our ck static lib
add_library(fallout2-ck STATIC
    src/ck_scripting.cc
    src/ck_rendering.cc

    src/ck_debug_overlay/ck_debug_overlay.cc
    src/ck_debug_overlay/ck_debug_overlay_render.cc
)

# includes relative to fallout2-ck
target_include_directories(fallout2-ck PUBLIC 
    include
    include/ck_debug_overlay

    src/ce_bridge

    src/vendor/luajit/src
    "${CMAKE_CURRENT_SOURCE_DIR}/fallout2-ce/src"
)

# link static luajit from vendor
target_link_libraries(fallout2-ck PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/luajit/src/libluajit.a"
    dl
)

# make submodule
add_subdirectory(fallout2-ce)

file(GLOB CE_BRIDGE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ce_bridge/*.cc"
)

target_include_directories(fallout2-ce PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(mapper-ce PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_sources(fallout2-ce PRIVATE ${CE_BRIDGE_SOURCES})
target_sources(mapper-ce PRIVATE ${CE_BRIDGE_SOURCES})
