#include <vector>
#include <unordered_set>

#include "ck_lua_proxy/ck_lua_proxy.h"
#include "ck_scripting.h"

#include "log.h"
static const Logger log("ck_mods.cc");

// LUA calls
namespace ck::proxy::detail {
    extern int reload_mods;
}

namespace ck::mods {
    // mod_id strings pool
    static std::unordered_set<std::string> g_immutable_string_pool;
    // mod_id pointers
    static std::vector<const char*> g_active_mods;
    // mod reload in progress
    static bool g_reloading_mods = false;

    const std::vector<const char*>& all() {
        return g_active_mods;
    }

    const char* ptr(const char* mod_id) {
        auto it = g_immutable_string_pool.find(mod_id);
        if (it == g_immutable_string_pool.end()) {
            return nullptr;
        }

        return it->c_str();
    }

    bool reload() {
        if (common::in_combat()) {
            common::print_monitor_message("Disabled in combat");

            return false;
        }

        log.header("Reloading mods");

        g_reloading_mods = true;
        ck::proxy::execute_proxy_call<bool>(ck::proxy::detail::reload_mods);
        g_reloading_mods = false;

        return true;
    }

    bool reloading() {
        return g_reloading_mods;
    }

    bool add(const char* mod_id) {
        if (mod_id == nullptr) return false;

        auto [it, inserted] = g_immutable_string_pool.insert(mod_id);
        const char* permanent_ptr = it->c_str();

        for (const char* active_mod_ptr : g_active_mods) {
            if (active_mod_ptr == permanent_ptr) {
                return false;
            }
        }

        g_active_mods.push_back(permanent_ptr);
        return true;
    }

    bool drop(const char* mod_id) {
        if (mod_id == nullptr) return false;
        const char* mod_id_ptr = ptr(mod_id);

        bool removed = (std::erase(g_active_mods, mod_id_ptr) > 0);

        removed ? log.debug("Removed mod_id: {}", mod_id) : log.error("Not found: {}", mod_id);
        return removed;
    }
}
