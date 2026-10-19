add_library(fallout2-ck STATIC
    system/init.cc
    system/hooks.cc
    system/rendering.cc
    system/i18n.cc
    system/encoding.cc
    system/input/input.cc

    debug/overlay.cc
    debug/hexes.cc
    debug/render.cc
    debug/object_format.cc

    engine/config/patch.cc
    engine/config/maps.cc
    engine/config/city.cc

    geometry/geometry.cc
    geometry/landscape.cc

    # Utils
    utils/fallout/types.cc

    utils/utils.cc
    utils/log.cc
    utils/ck_ids.cc

    ck_gameplay_common.cc
    # FFI
    ffi/validator.cc
    ffi/tests.cc
    ffi/system.cc
    ffi/gameplay.cc
    ffi/manifest.cc

    ck_mods/ck_mods.cc
    ck_state/ck_state.cc
    ck_dispatcher/ck_dispatcher.cc
    ck_messages/ck_messages.cc

    ck_lua_proxy/ck_lua_proxy.cc
    ck_lua_proxy/ck_lua_proxy_dispatcher.cc
    ck_lua_proxy/ck_lua_proxy_state.cc

    ck_registry/ck_registry.cc
    ck_registry/ck_registry_created.cc
    ck_registry/ck_registry_deleted.cc
    ck_registry/ck_registry_modified.cc

    ck_proto/cache/ck_proto_cache.cc
    ck_proto/cache/ck_proto_ffi.cc

    ck_proto/registry/ck_proto_registry.cc
    ck_proto/registry/ck_item_proto.cc

    ck_assets/ck_frm.cc
    ck_assets/ck_asset_registry.cc

    ck_knowledge/ck_knowledge.cc

    ck_area/ck_area.cc
    game_time/ck_game_time.cc
    map/ck_map.cc
    map/ck_map_batch.cc
    map/ck_map_camera_borders.cc
    script/ck_script.cc
    object/ck_object.cc
    object/critter/ck_critter.cc
    object/critter/ck_critter_proto.cc
    object/critter/ck_stats.cc
    object/critter/ck_skills.cc
    object/critter/ck_perks.cc
    object/ck_item.cc
)

target_precompile_headers(fallout2-ck PRIVATE
	<vector>
	<string>
	<format>
	<unordered_map>
	"${CMAKE_CURRENT_SOURCE_DIR}/../include/log.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../include/utils.h"
)

target_compile_features(fallout2-ck PRIVATE cxx_std_20)

target_include_directories(fallout2-ck PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/sqlite3"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/picojson/"
    $<TARGET_PROPERTY:SDL2-static,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(fallout2-ck PUBLIC
    sqlite3_vendor
    libluajit
    pthread
    dl
    SDL2-static
)

target_link_libraries(fallout2-ck PRIVATE fallout2-ce-alias)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(fallout2-ck PUBLIC $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer>)
endif()
