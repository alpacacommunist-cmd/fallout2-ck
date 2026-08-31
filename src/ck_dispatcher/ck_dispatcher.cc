#include "ck_dispatcher.h"
#include "ck_lua_proxy/ck_lua_proxy.h"
#include "ck_lua_proxy/ck_lua_proxy_dispatcher.h"
#include "object/ck_object.h"

#include <algorithm>
#include <string>
#include <vector>

#include "ck_log.h"
static const Logger log("CK Dispatcher");

namespace ck {
    int current_map_id();
}

const char* g_current_mod_id = nullptr;

// map update intervals
static int g_last_update_ticks = 0;
static const int MAP_UPDATE_INTERVAL_TICKS = 10;

static std::vector<std::string> g_active_mods;

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

    template<typename... Args>
        void emit(const char* event_name, Args... args) {
            if (!ck::proxy::is_ready() || !event_name) return;

            for (const auto& mod_id : g_active_mods) {
                log.debug("Emit event {} for {}", event_name, mod_id);

                ModContextGuard guard(mod_id.c_str());
                ck::proxy::emit_for_mod(mod_id.c_str(), event_name, args...);
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
            // return ck::proxy::on_proc(lua_id, proc_id, fixed_param, "unknown");
        }

        ModContextGuard guard(object_mod_id);
        bool result = ck::proxy::on_proc(lua_id, proc_id, fixed_param, object_mod_id);

        return result;
    }

    bool on_proto_proc(int pid, int proc_id, int fixed_param, const char* object_mod_id) {
        if (!object_mod_id) {
            log.warn("ck_dispatcher_on_proc called with null object_mod_id");
            // return ck::proxy::on_proc(lua_id, proc_id, fixed_param, "unknown");
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
        emit("onTimeAdvance", hours, minutes);
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

        emit("map_enter", ck::current_map_id());
    }
}

// ffi

bool ck_dispatcher_load_mod(const char* mod_id) {
	log.info("Loading mod: {}", mod_id);
	if (!ck::proxy::is_ready() || !mod_id) return false;

	std::string target_mod(mod_id);
	auto it = std::find(g_active_mods.begin(), g_active_mods.end(), target_mod);
	if (it == g_active_mods.end()) g_active_mods.push_back(target_mod);

	ModContextGuard guard(mod_id);
	if (!ck::proxy::load_mod(mod_id)) {
        log.error("Critical LuaJIT compilation error in mod '{}'", mod_id);

        g_active_mods.erase(
            std::remove(g_active_mods.begin(), g_active_mods.end(), target_mod),
            g_active_mods.end()
        );

        return false;
    }

	return true;
}

const char* ck_get_current_mod_id() {
    return g_current_mod_id;
}

void ck_dispatcher_emit_for_mod(const char* mod_id, const char* event_name) {
    if (!mod_id || !event_name) return;

    ModContextGuard guard(mod_id);
    ck::proxy::emit_for_mod(mod_id, event_name, ck::current_map_id());
}

