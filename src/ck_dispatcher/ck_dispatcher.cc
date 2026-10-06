#include "ck_dispatcher.h"
#include "ck_lua_proxy/ck_lua_proxy.h"
#include "ck_lua_proxy/ck_lua_proxy_dispatcher.h"
#include "object/ck_object.h"

#include <string>
#include <vector>
#include <unordered_set>

#include "ck_log.h"
static const Logger log("ck_dispatcher.cc");

namespace ck::common {
    int current_map_id();
}

const char* g_current_mod_id = nullptr;

// map update intervals
static int g_last_update_ticks = 0;
static const int MAP_UPDATE_INTERVAL_TICKS = 10;

// mod_id strings pool
static std::unordered_set<std::string> g_immutable_string_pool;
// mod_id pointers
static std::vector<const char*> g_active_mods;

static void ck_set_mod_context(const char* mod_id) {
	g_current_mod_id = mod_id;
}

struct ModContextGuard {
    const char* previous_context;

    ModContextGuard(const char* mod_id) {
        previous_context = g_current_mod_id;
        ck_set_mod_context(mod_id);
    }

    ~ModContextGuard() {
        ck_set_mod_context(previous_context);
    }
};

namespace ck::dispatcher {
    const char* current_mod_context() {
        return g_current_mod_id;
    }

    const char* mod_id_ptr(const char* mod_id) {
        auto it = g_immutable_string_pool.find(mod_id);
        if (it == g_immutable_string_pool.end()) {
            return nullptr;
        }

        return it->c_str();
    }

    template<typename... Args>
    void emit(const char* event_name, Args... args) {
        if (!ck::proxy::is_ready() || !event_name) return;

        for (const auto& mod_id : g_active_mods) {
            log.debug("Emit event {} for {}", event_name, mod_id);

            ModContextGuard guard(mod_id);
            ck::proxy::emit_for_mod(mod_id, event_name, args...);
        }
    }

    void on_map_update(int ticks) {
        if (ticks >= g_last_update_ticks && (ticks - g_last_update_ticks) < MAP_UPDATE_INTERVAL_TICKS) return;
        g_last_update_ticks = ticks;

        ck::proxy::on_map_update(ticks);
    }

    bool on_proc(int lua_id, int proc_id, int fixed_param, const char* object_mod_id) {
        if (!object_mod_id) {
            log.warn("ck_dispatcher_on_proc called with null object_mod_id");
        }

        ModContextGuard guard(object_mod_id);
        bool result = ck::proxy::on_proc(lua_id, proc_id, fixed_param, object_mod_id);

        return result;
    }

    bool on_proto_proc(int pid, int proc_id, int fixed_param, const char* object_mod_id) {
        if (!object_mod_id) {
            log.warn("ck_dispatcher_on_proc called with null object_mod_id");
        }

        ModContextGuard guard(object_mod_id);
        bool result = ck::proxy::on_proto_proc(pid, proc_id, fixed_param, object_mod_id);

        return result;
    }

    void on_game_start() {
        emit("onGameStart");
    }

    void on_engine_ready() {
        emit("onEngineReady");
    }

    void on_game_loaded() {
        emit("onGameLoaded");
    }

    void on_time_advance(int hours, int minutes) {
        emit("time_advance", hours, minutes);
    }

    void on_skill_used(int skill, int success_count, int bonus) {
        emit("skill_used", skill, success_count, bonus);
    }

    void on_critter_killed(const CkObjectFFI* victim, const CkObjectFFI* killer) {
        ck::proxy::critter_killed(victim, killer);
    }

    void on_day_passed() {
        emit("onDayPassed");
    }

    void on_map_enter() {
        log.debug("ck_dispatcher_on_map_enter");
        g_last_update_ticks = 0;

        emit("map_enter", ck::common::current_map_id());
    }
}

// FFI

bool ck_set_current_mod_context(const char* mod_id) {
    if (mod_id == nullptr) {
        ck_set_mod_context(nullptr);
        return true;
    }

    const char* mod_id_ptr = ck::dispatcher::mod_id_ptr(mod_id);
    if (mod_id_ptr != nullptr) {
        ck_set_mod_context(mod_id_ptr);
        return true;
    }

    return false;
}

bool ck_dispatcher_add_mod(const char* mod_id) {
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

bool ck_dispatcher_remove_mod(const char* mod_id) {
    if (mod_id == nullptr) return false;
    const char* mod_id_ptr = ck::dispatcher::mod_id_ptr(mod_id);

    bool removed = (std::erase(g_active_mods, mod_id_ptr) > 0);

    removed ? log.debug("Removed mod_id: {}", mod_id) : log.error("Not found: {}", mod_id);
    return removed;
}

void ck_dispatcher_emit_for_mod(const char* mod_id, const char* event_name) {
    if (!mod_id || !event_name) return;

    const char* mod_id_ptr = ck::dispatcher::mod_id_ptr(mod_id);
    if (mod_id_ptr == nullptr) return;

    ModContextGuard guard(mod_id_ptr);
    ck::proxy::emit_for_mod(mod_id_ptr, event_name, ck::common::current_map_id());
}

