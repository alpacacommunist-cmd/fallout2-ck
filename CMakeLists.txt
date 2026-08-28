# fallout2-ck/CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(fallout2-ck C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(ExternalProject)
find_package(SDL2 REQUIRED)

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
    src/geometry/geometry.cc
    src/geometry/landscape.cc

    src/ck_ffi_manifest.cc
    src/ck_log.cc
    src/ck_utils.cc
    src/ck_ids.cc
    src/ck_scripting.cc
    src/ck_rendering.cc
    src/ck_state/ck_state.cc
    src/ck_dispatcher/ck_dispatcher.cc
    src/ck_messages/ck_messages.cc
    src/ck_lua_proxy/ck_lua_proxy.cc
    src/ck_lua_proxy/ck_lua_proxy_dispatcher.cc
    src/ck_lua_proxy/ck_lua_proxy_state.cc

    src/ce_config/ck_config_patch.cc
    src/ce_config/ck_config_maps.cc
    src/ce_config/ck_config_city.cc
    src/ce_config/ck_encoding.cc

    src/ck_registry/ck_registry.cc
    src/ck_registry/ck_registry_created.cc
    src/ck_registry/ck_registry_deleted.cc
    src/ck_registry/ck_registry_modified.cc

    src/vendor/sqlite3/sqlite3.c

    src/ck_proto/cache/ck_proto_cache.cc
    src/ck_proto/cache/ck_proto_ffi.cc

    src/ck_proto/registry/ck_proto_registry.cc
    src/ck_proto/registry/ck_proto_item.cc

    src/ck_assets/ck_frm.cc
    src/ck_assets/ck_asset_registry.cc

    src/ck_input/ck_input.cc
    src/ck_i18n/ck_i18n.cc

    src/ck_knowledge/ck_knowledge.cc

    src/ck_area/ck_area.cc
    src/game_time/ck_game_time.cc
    src/map/ck_map.cc
    src/map/ck_map_batch.cc
    src/map/ck_map_camera_borders.cc
    src/script/ck_script.cc
    src/object/ck_object.cc
    src/object/critter/ck_critter.cc
    src/object/critter/ck_critter_proto.cc
    src/object/critter/ck_stats.cc
    src/object/critter/ck_skills.cc
    src/object/critter/ck_perks.cc
    src/object/ck_item.cc

    src/ck_debug_overlay/ck_debug_overlay.cc
    src/ck_debug_overlay/ck_debug_overlay_hexes.cc
    src/ck_debug_overlay/ck_debug_overlay_render.cc
    src/ck_debug_overlay/ck_debug_object_format.cc
)

target_compile_features(fallout2-ck PRIVATE cxx_std_20)
add_dependencies(fallout2-ck luajit_project)

target_include_directories(fallout2-ck PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/"
    # "${CMAKE_CURRENT_SOURCE_DIR}/src/ce_bridge/"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/sqlite3"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/picojson/"
    "${LUAJIT_INCLUDE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/fallout2-ce/src/"
     ${SDL2_INCLUDE_DIRS}
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_source_files_properties(src/vendor/sqlite3/sqlite3.c PROPERTIES COMPILE_FLAGS "-w")
elseif(MSVC)
    set_source_files_properties(src/vendor/sqlite3/sqlite3.c PROPERTIES COMPILE_FLAGS "/W0")
endif()

target_link_libraries(fallout2-ck PUBLIC 
    ${LUAJIT_LIB}
    pthread
    dl
)

# set(CE_BRIDGE_SOURCES
#     "${CMAKE_CURRENT_SOURCE_DIR}/src/ce_bridge/ck_input.cc"
# )

foreach(TARGET fallout2-ce mapper-ce)
    if(TARGET ${TARGET})
	# target_sources(${TARGET} PRIVATE ${CE_BRIDGE_SOURCES})
        target_include_directories(${TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
        target_link_libraries(${TARGET} fallout2-ck)
        set_target_properties(${TARGET} PROPERTIES ENABLE_EXPORTS TRUE)
    endif()
endforeach()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(fallout2-ck PUBLIC $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer>)

    foreach(TARGET fallout2-ce mapper-ce)
        if(TARGET ${TARGET})
            target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer>)
            target_link_options(${TARGET} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
        endif()
    endforeach()
endif()
