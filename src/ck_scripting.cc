#include "ck_scripting.h"
#include "ck_lua_proxy/ck_lua_proxy.h"

#include "log.h"
static logging::Context logger{"ck_scripting.cc", logging::Source::Native};

namespace ck {
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
