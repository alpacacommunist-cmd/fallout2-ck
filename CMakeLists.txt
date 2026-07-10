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

option(ENABLE_PROTO_CACHE "Enable SQLite3-based proto cache and GUI features" OFF)

add_subdirectory(fallout2-ce)

add_library(fallout2-ck STATIC
    src/geometry/geometry.cc
    src/geometry/landscape.cc

    src/ck_ffi_manifest.cc
    src/ck_scripting.cc
    src/ck_rendering.cc
    src/ck_log.cc
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

    src/ck_assets/ck_frm.cc
    src/ck_assets/ck_asset_registry.cc

    src/ck_area/ck_area.cc
    src/game_time/ck_game_time.cc
    src/map/ck_map.cc
    src/script/ck_script.cc
    src/object/ck_object.cc
    src/object/ck_object_registry.cc
    src/object/critter/ck_critter.cc
    src/object/critter/ck_stats.cc
    src/object/ck_item.cc

    src/ck_debug_overlay/ck_debug_overlay.cc
    src/ck_debug_overlay/ck_debug_overlay_hexes.cc
    src/ck_debug_overlay/ck_debug_overlay_render.cc
)
target_compile_features(fallout2-ck PRIVATE cxx_std_20)

add_dependencies(fallout2-ck luajit_project)

target_include_directories(fallout2-ck PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ce_bridge/"
    "${LUAJIT_INCLUDE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/picojson/"
    "${CMAKE_CURRENT_SOURCE_DIR}/fallout2-ce/src/"
)

if(ENABLE_PROTO_CACHE)
	target_sources(fallout2-ck PRIVATE
		src/vendor/sqlite3/sqlite3.c
		src/ck_assets/ck_proto_cache.cc
	)

	target_include_directories(fallout2-ck PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/sqlite3"
	)

	target_compile_definitions(fallout2-ck PRIVATE USE_PROTO_CACHE)
	target_link_libraries(fallout2-ck PUBLIC pthread)

	if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		set_source_files_properties(src/vendor/sqlite3/sqlite3.c PROPERTIES COMPILE_FLAGS "-w")
	elseif(MSVC)
		set_source_files_properties(src/vendor/sqlite3/sqlite3.c PROPERTIES COMPILE_FLAGS "/W0")
	endif()
endif()

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
