# fallout2-ck/CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(fallout2-ck C CXX)

include(ExternalProject)
set(LUAJIT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/luajit")
set(LUAJIT_INCLUDE "${LUAJIT_SRC_DIR}/src")
set(LUAJIT_LIB "${LUAJIT_SRC_DIR}/src/libluajit.a")

ExternalProject_Add(luajit_project
    SOURCE_DIR "${LUAJIT_SRC_DIR}"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make -C "${LUAJIT_SRC_DIR}/src"
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS "${LUAJIT_LIB}"
)

add_subdirectory(fallout2-ce)

add_library(fallout2-ck STATIC
    src/vendor/sqlite3/sqlite3.c

    src/ck_scripting.cc
    src/ck_rendering.cc

    src/ce_config/ck_config_patch.cc
    src/ce_config/ck_message_patch.cc
    src/ce_config/ck_map_registry.cc
    src/ce_config/ck_map_patch.cc

    src/ck_assets/ck_frm.cc
    src/ck_assets/ck_asset_registry.cc
    src/ck_assets/ck_proto_cache.cc

    src/rendering/rendering_bindings.cc
    src/ck_assets/assets_bindings.cc

    src/game_time/ck_game_time.cc
    src/map/ck_map.cc
    src/script/ck_script.cc
    src/object/ck_object.cc
    src/object/ck_object_registry.cc

    src/ck_debug_overlay/ck_debug_overlay.cc
    src/ck_debug_overlay/ck_debug_overlay_hexes.cc
    src/ck_debug_overlay/ck_debug_overlay_render.cc
)

add_dependencies(fallout2-ck luajit_project)

target_include_directories(fallout2-ck PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ce_bridge"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/sqlite3"
    "${LUAJIT_INCLUDE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/fallout2-ce/src"
)

target_link_libraries(fallout2-ck PUBLIC 
    ${LUAJIT_LIB}
    dl
)

set(CE_BRIDGE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ce_bridge/ck_input.cc"
)

foreach(TARGET fallout2-ce mapper-ce)
    if(TARGET ${TARGET})
        target_sources(${TARGET} PRIVATE ${CE_BRIDGE_SOURCES})
        target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
        target_link_libraries(${TARGET} fallout2-ck)

	set_target_properties(${TARGET} PROPERTIES ENABLE_EXPORTS TRUE) # ffi
    endif()
endforeach()
