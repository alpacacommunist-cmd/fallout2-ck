#include "ck_dispatcher.h"
#include "ck_lua_proxy/ck_lua_proxy.h"
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

static void lua_push_arg(lua_State* L, int val) { lua_pushinteger(L, val); }
static void lua_push_arg(lua_State* L, unsigned int val) { lua_pushinteger(L, val); }
static void lua_push_arg(lua_State* L, double val) { lua_pushnumber(L, val); }
static void lua_push_arg(lua_State* L, const char* val) { lua_pushstring(L, val); }
static void lua_push_arg(lua_State* L, const std::string& val) { lua_pushstring(L, val.c_str()); }
static void lua_push_arg(lua_State* L, bool val) { lua_pushboolean(L, val); }

template<typename... Args>
static void lua_push_args_chain(lua_State* L, Args... args) {
    (lua_push_arg(L, args), ...);
}

template<typename... Args>
static void invoke_lua_emit(const char* mod_id, const char* event_name, Args... args) {
    lua_rawgeti(gLuaState, LUA_REGISTRYINDEX, ck::proxy::emit_for_mod);
    lua_pushstring(gLuaState, mod_id);
    lua_pushstring(gLuaState, event_name);

    lua_push_args_chain(gLuaState, args...);

    int total_args = 2 + sizeof...(Args);

    if (lua_pcall(gLuaState, total_args, 0, 0) != LUA_OK) {
        log.error("Error routing event '{}:{}': {}", mod_id, event_name, lua_tostring(gLuaState, -1));
        lua_pop(gLuaState, 1);
    }
}

template<typename ReturnType, typename... Args>
ReturnType ck_dispatcher_call(int func_ref, Args... args) {
	if (!gLuaState || func_ref == LUA_NOREF) return ReturnType{};

	lua_rawgeti(gLuaState, LUA_REGISTRYINDEX, func_ref);
	(lua_push_arg(gLuaState, args), ...);

	int total_args = sizeof...(Args);
	if (!safe_pcall_with_traceback(gLuaState, total_args, 1)) {
		log.error("Runtime error during dispatcher call");
		lua_pop(gLuaState, 1);
		return ReturnType{};
	}

	ReturnType result{};
	if constexpr (std::is_same_v<ReturnType, int>) result = static_cast<int>(lua_tointeger(gLuaState, -1));
	else if constexpr (std::is_same_v<ReturnType, bool>) result = lua_toboolean(gLuaState, -1);
	else if constexpr (std::is_same_v<ReturnType, std::string>) { if (lua_isstring(gLuaState, -1)) result = lua_tostring(gLuaState, -1); }

	lua_pop(gLuaState, 1);
	return result;
}

template<typename... Args>
void ck_dispatcher_emit(const char* event_name, Args... args) {
	if (!gLuaState || ck::proxy::emit_for_mod == LUA_NOREF || !event_name) return;

	std::string previous_context = g_current_mod_id;

	for (const auto& mod_id : g_active_mods) {
		log.info("Emit event {} for {}", event_name, mod_id);

		ck_set_mod_context(mod_id.c_str());

		invoke_lua_emit(mod_id.c_str(), event_name, args...);
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
	if (!gLuaState || ck::proxy::on_proc == LUA_NOREF) return false;

	const char* previous_context = g_current_mod_id;

	ck_set_mod_context(object_mod_id);

	lua_rawgeti(gLuaState, LUA_REGISTRYINDEX, ck::proxy::on_proc);
	lua_pushinteger(gLuaState, lua_id);
	lua_pushinteger(gLuaState, proc_id);

	if (lua_pcall(gLuaState, 2, 1, 0) != LUA_OK) {
		log.error("Runtime error in ck_on_proc: {}", lua_tostring(gLuaState, -1));
		lua_pop(gLuaState, 1);

		ck_set_mod_context(previous_context);
		return false;
	}

	bool result = false;
	if (lua_isboolean(gLuaState, -1)) result = lua_toboolean(gLuaState, -1);

	lua_pop(gLuaState, 1);

	ck_set_mod_context(previous_context);

	return result;
}

void ck_dispatcher_on_game_start() {
	log.debug("ck_dispatcher_on_game_start");
	ck_dispatcher_emit("onGameStart");
}

void ck_dispatcher_on_game_loaded() {
	log.debug("ck_dispatcher_on_game_loaded");
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

	clear_lua_registry(ck::proxy::clear_tracked_objects, "state.clear_tracked_objects");
	clear_lua_registry(ck::proxy::clear_registry, "objects.clear_registry");

	g_last_update_ticks = 0;

	ck_dispatcher_emit("onMapEnter");
}

int ck_dispatcher_get_sync_load_ref() { return ck::proxy::state_sync_load; }
int ck_dispatcher_get_sync_save_ref() { return ck::proxy::state_sync_save; }

// ffi

int ck_dispatcher_get_state_tile(int map_id, const char* lua_tag) {
	return ck_dispatcher_call<int>(ck::proxy::get_state_tile, g_current_mod_id, map_id, lua_tag);
}

bool ck_dispatcher_load_mod(const char* mod_id) {
	log.info("mod_id: {}", mod_id);
	if (!gLuaState || ck::proxy::load_and_init_mod == LUA_NOREF || !mod_id) return false;

	std::string target_mod(mod_id);
	auto it = std::find(g_active_mods.begin(), g_active_mods.end(), target_mod);
	if (it == g_active_mods.end()) {
		g_active_mods.push_back(target_mod);
	}

	std::string previous_context = g_current_mod_id;
	ck_set_mod_context(mod_id);

	lua_rawgeti(gLuaState, LUA_REGISTRYINDEX, ck::proxy::load_and_init_mod);
	lua_pushstring(gLuaState, mod_id);

	if (lua_pcall(gLuaState, 1, LUA_MULTRET, 0) != LUA_OK) {
        log.error("Critical LuaJIT compilation error in mod '{}': {}", mod_id, lua_tostring(gLuaState, -1));
        lua_pop(gLuaState, 1);

        g_active_mods.erase(
            std::remove(g_active_mods.begin(), g_active_mods.end(), target_mod),
            g_active_mods.end()
        );

        ck_set_mod_context(previous_context.c_str());
        return false;
    }

	lua_pop(gLuaState, 1);

	ck_set_mod_context(previous_context.c_str());
	return true;
}

const char* ck_get_current_mod_id() {
	return g_current_mod_id;
}

void ck_dispatcher_emit_for_mod(const char* mod_id, const char* event_name) {
	if (!gLuaState || ck::proxy::emit_for_mod == LUA_NOREF || !mod_id || !event_name) return;

	std::string previous_context = g_current_mod_id;
	ck_set_mod_context(mod_id);

	invoke_lua_emit(mod_id, event_name);

	ck_set_mod_context(previous_context.c_str());
}

