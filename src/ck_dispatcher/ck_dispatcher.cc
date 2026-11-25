#include "ck_dispatcher.h"
#include "lua/proxy/proxy.h"
#include "lua/proxy/dispatcher.h"
#include "object/ck_object.h"
#include "runtime/mods.h"

#include "log.h"
static constexpr logging::Context logger{"dispatcher.cc"};

namespace ck::common {
    int current_map_id();
}

namespace ck::dispatcher {
    template<typename... Args>
    void emit(const char* event_name, Args... args) {
        if (!ck::proxy::is_ready() || !event_name) return;

        ck::proxy::emit(event_name, args...);
    }

    void emit_for_mod(const char* mod_id, const char* event_name) {
        if (!mod_id || !event_name) return;

        const char* mod_id_ptr = ck::mods::ptr(mod_id);
        if (mod_id_ptr == nullptr) return;

        ck::proxy::emit_for_mod(mod_id_ptr, event_name, ck::common::current_map_id());
    }

    void on_map_update(int ticks) {
        ck::proxy::on_map_update(ticks);
    }

    bool on_proc(int lua_id, int proc_id, int param, const char* object_mod_id) {
        if (!object_mod_id) {
            logger.warning("ck_dispatcher_on_proc called with null object_mod_id");
        }

        bool result = ck::proxy::on_proc(lua_id, proc_id, param, object_mod_id);

        return result;
    }

    bool on_proto_proc(int proc_id, int param, fallout::Object* object) {
        bool result = ck::proxy::on_proto_proc(proc_id, param, object);

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

    void on_critter_killed(fallout::Object* victim, fallout::Object* killer) {
        ck::proxy::critter_killed(victim, killer);
    }

    void on_day_passed() {
        emit("onDayPassed");
    }

    void on_map_enter() {
        logger.debug("ck_dispatcher_on_map_enter");

        emit("map_enter", ck::common::current_map_id());
    }
}

