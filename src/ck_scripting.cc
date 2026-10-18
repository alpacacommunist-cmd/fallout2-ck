#include "ck_scripting.h"
#include "ck_mods/ck_mods.h"

#include "ck_registry/ck_registry.h"
#include "ck_lua_proxy/ck_lua_proxy.h"

#include "log.h"
static logging::Context logger{"ck_scripting.cc", logging::Source::Native};

namespace ck {
    // ck_critter.cc
    namespace critter::proto {
        void clear_prototypes_for_mod(const char* mod_id);
    }
    // map/ck_map_camera_borders.cc
    namespace map::borders {
        void clear_for_mod(const char* mod_id);
    }
    // ck_rendering.cc
    namespace rendering {
        void clear_for_mod(const char* mod_id);
    }
    // ck_lua_proxy.cc
    namespace proxy::detail {
        extern int clear_registries;
        extern int map_exit;
    }
}

namespace ck::common {
    void clear_lua_registries() {
        ck::proxy::execute_proxy_call<bool>(ck::proxy::detail::clear_registries);
    }

    void lua_map_exit() {
        ck::proxy::execute_proxy_call<bool>(ck::proxy::detail::map_exit);
    }
}

// TODO: move to mods.cc
void ck_clear_mod_resources(const char* mod_id) {
    const char* mod_id_ptr = ck::mods::ptr(mod_id);

    ck::registry::clear_resources_for_mod(mod_id_ptr);
    ck::critter::proto::clear_prototypes_for_mod(mod_id_ptr);
    ck::map::borders::clear_for_mod(mod_id_ptr);
    ck::rendering::clear_for_mod(mod_id_ptr);
}


// FFI

// TODO: move to mods.cc
void ck_mods_clear_resources_for_mod(const char* mod_id) {
    ck_clear_mod_resources(mod_id);
}
