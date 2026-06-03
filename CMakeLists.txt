# fallout2-ck/CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(fallout2-ck CXX)

add_subdirectory(fallout2-ce)

set(LUAJIT_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/luajit/src")
set(LUAJIT_LIB "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/luajit/src/libluajit.a")

add_library(fallout2-ck STATIC
    src/ck_scripting.cc
    src/ck_rendering.cc

    src/game_time/game_time_bindings.cc
    src/rendering/rendering_bindings.cc
    src/map/map_bindings.cc

    src/game_time/ck_game_time.cc
    src/map/ck_map.cc

    src/ck_debug_overlay/ck_debug_overlay.cc
    src/ck_debug_overlay/ck_debug_overlay_hexes.cc
    src/ck_debug_overlay/ck_debug_overlay_render.cc
)

target_include_directories(fallout2-ck PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ck_debug_overlay"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ce_bridge"
    "${LUAJIT_INCLUDE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/fallout2-ce/src"
)

target_link_libraries(fallout2-ck PUBLIC 
    ${LUAJIT_LIB}
    dl
)

file(GLOB CE_BRIDGE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/ce_bridge/*.cc")

foreach(TARGET fallout2-ce mapper-ce)
    if(TARGET ${TARGET})
        target_sources(${TARGET} PRIVATE ${CE_BRIDGE_SOURCES})

        target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

        target_link_libraries(${TARGET} fallout2-ck)
    endif()
endforeach()
