#include "ck_dispatcher.h"
#include "ck_lua_proxy/ck_lua_proxy.h"
#include "ck_lua_proxy/ck_lua_proxy_dispatcher.h"
#include "ck_utils.h"

#include "ck_log.h"
static const Logger log("CK Dispatcher");

const char* g_current_mod_id = "unknown";

// map update intervals
static int g_last_update_ticks = 0;
static const int MAP_UPDATE_INTERVAL_TICKS = 10;

static std::vector<std::string> g_active_mods;

static void ck_set_mod_context(const char* mod_id) {
	g_current_mod_id = mod_id ? mod_id : "unknown";
}

static void clear_lua_registry(int ref, const char* name) {
    if (ref == LUA_NOREF) return;

    lua_rawgeti(gLuaState, LUA_REGISTRYINDEX, ref);
    if (lua_pcall(gLuaState, 0, 0, 0) != LUA_OK) {
        log.error("Error in {}: {}", name, lua_tostring(gLuaState, -1));
        lua_pop(gLuaState, 1);
    }
}

template<typename... Args>
void ck_dispatcher_emit(const char* event_name, Args... args) {
	if (!ck::proxy::is_ready() || !event_name) return;

	std::string previous_context = g_current_mod_id;

	for (const auto& mod_id : g_active_mods) {
		log.info("Emit event {} for {}", event_name, mod_id);

		ck_set_mod_context(mod_id.c_str());

		ck::proxy::execute_emit_for_mod(mod_id.c_str(), event_name, args...);
	}

	ck_set_mod_context(previous_context.c_str());
}

void ck_dispatcher_on_map_update(int ticks) {
	if (!ck::proxy::is_ready()) return;

	if (ticks >= g_last_update_ticks && (ticks - g_last_update_ticks) < MAP_UPDATE_INTERVAL_TICKS) return;
	g_last_update_ticks = ticks;

	ck::proxy::execute_map_update(ticks);
}

bool ck_dispatcher_on_proc(int lua_id, int proc_id, const char* object_mod_id) {
	if (!ck::proxy::is_ready()) return false;

	const char* previous_context = g_current_mod_id;
	ck_set_mod_context(object_mod_id);

	bool result = ck::proxy::execute_on_proc(lua_id, proc_id, object_mod_id);

	ck_set_mod_context(previous_context);
	return result;
}

void ck_dispatcher_on_game_start() {
	ck_dispatcher_emit("onGameStart");
}

void ck_dispatcher_on_game_loaded() {
	ck_dispatcher_emit("onGameLoaded");
}

void ck_dispatcher_on_time_advance(int hours, int minutes) {
	ck_dispatcher_emit("onTimeAdvance", hours, minutes);
};

void ck_dispatcher_on_day_passed() {
	ck_dispatcher_emit("onDayPassed");
};

void ck_dispatcher_on_map_enter() {
	log.debug("ck_dispatcher_on_map_enter");

	gObjectRegistry.clear();

	ck::proxy::execute_clear_registry_fn(ck::proxy::clear_tracked_objects);
	ck::proxy::execute_clear_registry_fn(ck::proxy::clear_registry);

	g_last_update_ticks = 0;
	ck_dispatcher_emit("onMapEnter");
}

int ck_dispatcher_get_sync_load_ref() { return ck::proxy::state_sync_load; }
int ck_dispatcher_get_sync_save_ref() { return ck::proxy::state_sync_save; }

// ffi

bool ck_dispatcher_load_mod(const char* mod_id) {
	log.info("mod_id: {}", mod_id);
	if (!ck::proxy::is_ready() || !mod_id) return false;

	std::string target_mod(mod_id);
	auto it = std::find(g_active_mods.begin(), g_active_mods.end(), target_mod);
	if (it == g_active_mods.end()) g_active_mods.push_back(target_mod);

	std::string previous_context = g_current_mod_id;
	ck_set_mod_context(mod_id);

	if (!ck::proxy::execute_load_mod(mod_id)) {
        log.error("Critical LuaJIT compilation error in mod '{}'", mod_id);

        g_active_mods.erase(
            std::remove(g_active_mods.begin(), g_active_mods.end(), target_mod),
            g_active_mods.end()
        );

        ck_set_mod_context(previous_context.c_str());
        return false;
    }

	ck_set_mod_context(previous_context.c_str());
	return true;
}

const char* ck_get_current_mod_id() {
	return g_current_mod_id;
}

void ck_dispatcher_emit_for_mod(const char* mod_id, const char* event_name) {
	if (!mod_id || !event_name) return;

	std::string previous_context = g_current_mod_id;
	ck_set_mod_context(mod_id);

	ck::proxy::execute_emit_for_mod(mod_id, event_name);

	ck_set_mod_context(previous_context.c_str());
}

