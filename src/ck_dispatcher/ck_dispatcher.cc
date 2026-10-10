#include "ck_dispatcher.h"
#include "ck_lua_proxy/ck_lua_proxy.h"
#include "ck_lua_proxy/ck_lua_proxy_dispatcher.h"
#include "object/ck_object.h"
#include "ck_mods/ck_mods.h"

#include "log.h"
static constexpr logging::Context logger{"dispatcher.cc"};

namespace ck::common {
    int current_map_id();
}

static const char* g_current_mod_id = nullptr;

// map update intervals
static int g_last_update_ticks = 0;
static const int MAP_UPDATE_INTERVAL_TICKS = 10;

namespace ck::dispatcher {
    bool set_context(const char* mod_id) {
        // Means no context
        if (mod_id == nullptr) {
            g_current_mod_id = nullptr;
            return true;
        }

        // MOD context
        const char* mod_id_ptr = ck::mods::ptr(mod_id);
        if (mod_id_ptr != nullptr) {
            g_current_mod_id = mod_id_ptr;
            return true;
        }

        // Couldn't set context (context unknown)
        return false;
    }

    const char* current_context() {
        return g_current_mod_id;
    }

    struct ModContextGuard {
        const char* previous_context;

        ModContextGuard(const char* mod_id) {
            previous_context = g_current_mod_id;
            set_context(mod_id);
        }

        ~ModContextGuard() {
            set_context(previous_context);
        }
    };

    template<typename... Args>
    void emit(const char* event_name, Args... args) {
        if (!ck::proxy::is_ready() || !event_name) return;

        for (const auto& mod_id : ck::mods::all()) {
            logger.debug("Emit event {} for {}", event_name, mod_id);

            ModContextGuard guard(mod_id);
            ck::proxy::emit_for_mod(mod_id, event_name, args...);
        }
    }

    void emit_for_mod(const char* mod_id, const char* event_name) {
        if (!mod_id || !event_name) return;

        const char* mod_id_ptr = ck::mods::ptr(mod_id);
        if (mod_id_ptr == nullptr) return;

        ModContextGuard guard(mod_id_ptr);
        ck::proxy::emit_for_mod(mod_id_ptr, event_name, ck::common::current_map_id());
    }

    void on_map_update(int ticks) {
        int delta = ticks - g_last_update_ticks;
        if (ticks >= g_last_update_ticks && delta < MAP_UPDATE_INTERVAL_TICKS) {
            return;
        }

        g_last_update_ticks = ticks;
        ck::proxy::on_map_update(ticks);
    }

    bool on_proc(int lua_id, int proc_id, int fixed_param, const char* object_mod_id) {
        if (!object_mod_id) {
            logger.warning("ck_dispatcher_on_proc called with null object_mod_id");
        }

        ModContextGuard guard(object_mod_id);
        bool result = ck::proxy::on_proc(lua_id, proc_id, fixed_param, object_mod_id);

        return result;
    }

    bool on_proto_proc(int pid, int proc_id, int fixed_param, const char* object_mod_id) {
        if (!object_mod_id) {
            logger.warning("ck_dispatcher_on_proc called with null object_mod_id");
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
        logger.debug("ck_dispatcher_on_map_enter");
        g_last_update_ticks = 0;

        emit("map_enter", ck::common::current_map_id());
    }
}

