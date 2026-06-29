#include "ck_dispatcher.h"

#include "ck_log.h"
static const Logger log("CK Dispatcher");

static lua_State* g_L = nullptr;

// lua function refs
static int g_emit_for_mod_ref  = LUA_NOREF;
static int g_on_map_update_ref = LUA_NOREF;
static int g_on_proc_ref       = LUA_NOREF;
static int g_clear_tracked_objects_ref = LUA_NOREF;
static int g_clear_registry_ref = LUA_NOREF;
static int g_load_and_init_mod_ref = LUA_NOREF;

static std::vector<std::string> g_active_mods;
static const char* g_current_mod_id = "unknown";

static int cache_module_function(lua_State* L, const char* module_name, const char* function_name) {
	lua_getglobal(L, "require");
	lua_pushstring(L, module_name);

	if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
		log.error("Failed to require module '{}': {}", module_name, lua_tostring(L, -1));
		lua_pop(L, 1);
		return LUA_NOREF;
	}

	lua_getfield(L, -1, function_name);
	if (!lua_isfunction(L, -1)) {
		log.error("Function '{}' not found in module '{}'!", function_name, module_name);
		lua_pop(L, 2);
		return LUA_NOREF;
	}

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	lua_pop(L, 1);
	return ref;
}

static void ck_set_mod_context(const char* mod_id) {
	g_current_mod_id = mod_id ? mod_id : "unknown";
}

static void clear_lua_registry(int ref, const char* name) {
    if (ref == LUA_NOREF) return;

    lua_rawgeti(g_L, LUA_REGISTRYINDEX, ref);
    if (lua_pcall(g_L, 0, 0, 0) != LUA_OK) {
        log.error("Error in {}: {}", name, lua_tostring(g_L, -1));
        lua_pop(g_L, 1);
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
    lua_rawgeti(g_L, LUA_REGISTRYINDEX, g_emit_for_mod_ref);
    lua_pushstring(g_L, mod_id);
    lua_pushstring(g_L, event_name);

    lua_push_args_chain(g_L, args...);

    int total_args = 2 + sizeof...(Args);

    if (lua_pcall(g_L, total_args, 0, 0) != LUA_OK) {
        log.error("Error routing event '{}:{}': {}", mod_id, event_name, lua_tostring(g_L, -1));
        lua_pop(g_L, 1);
    }
}

void ck_dispatcher_init(lua_State* L) {
	g_L = L;
	g_current_mod_id = "unknown";

	g_load_and_init_mod_ref     = cache_module_function(g_L, "ck.system.loader", "load_and_init_mod");
	g_emit_for_mod_ref          = cache_module_function(g_L, "ck.system.events", "emit_for_mod");
	g_on_map_update_ref         = cache_module_function(g_L, "ck.system.events", "ck_on_map_update");
	g_on_proc_ref               = cache_module_function(g_L, "ck.system.events", "ck_on_proc");
	g_clear_tracked_objects_ref = cache_module_function(g_L, "ck.fallout2.state",  "clear_tracked_objects");
	g_clear_registry_ref        = cache_module_function(g_L, "ck.fallout2.objects", "clear_registry");

	log.info("Dispatcher successfully initialized and cached Lua hooks.");
}

void ck_dispatcher_shutdown() {
	if (g_L) {
		if (g_emit_for_mod_ref != LUA_NOREF)  luaL_unref(g_L, LUA_REGISTRYINDEX, g_emit_for_mod_ref);
		if (g_on_map_update_ref != LUA_NOREF) luaL_unref(g_L, LUA_REGISTRYINDEX, g_on_map_update_ref);
		if (g_on_proc_ref != LUA_NOREF)       luaL_unref(g_L, LUA_REGISTRYINDEX, g_on_proc_ref);
		if (g_clear_tracked_objects_ref != LUA_NOREF) luaL_unref(g_L, LUA_REGISTRYINDEX, g_clear_tracked_objects_ref);
		if (g_clear_registry_ref != LUA_NOREF) luaL_unref(g_L, LUA_REGISTRYINDEX, g_clear_registry_ref);


		g_emit_for_mod_ref = LUA_NOREF;
		g_on_map_update_ref = LUA_NOREF;
		g_on_proc_ref = LUA_NOREF;
		g_clear_tracked_objects_ref = LUA_NOREF;
		g_clear_registry_ref = LUA_NOREF;
	}

	g_active_mods.clear();
	g_L = nullptr;
}

template<typename... Args>
void ck_dispatcher_emit(const char* event_name, Args... args) {
	if (!g_L || g_emit_for_mod_ref == LUA_NOREF || !event_name) return;

	std::string previous_context = g_current_mod_id;

	for (const auto& mod_id : g_active_mods) {
		log.info("Emit event {} for {}", event_name, mod_id);

		ck_set_mod_context(mod_id.c_str());

		invoke_lua_emit(mod_id.c_str(), event_name, args...);
	}

	ck_set_mod_context(previous_context.c_str());
}

void ck_dispatcher_on_map_update(int ticks) {
	if (!g_L || g_on_map_update_ref == LUA_NOREF) return;

	lua_rawgeti(g_L, LUA_REGISTRYINDEX, g_on_map_update_ref);

	lua_pushinteger(g_L, ticks);

	if (lua_pcall(g_L, 1, 0, 0) != LUA_OK) {
		log.error("Error in ck_on_map_update: {}", lua_tostring(g_L, -1));
		lua_pop(g_L, 1);
	}
}

bool ck_dispatcher_on_proc(int lua_id, int proc_id, const char* object_mod_id) {
	if (!g_L || g_on_proc_ref == LUA_NOREF) return false;

	const char* previous_context = g_current_mod_id;

	ck_set_mod_context(object_mod_id);

	lua_rawgeti(g_L, LUA_REGISTRYINDEX, g_on_proc_ref);
	lua_pushinteger(g_L, lua_id);
	lua_pushinteger(g_L, proc_id);

	if (lua_pcall(g_L, 2, 1, 0) != LUA_OK) {
		log.error("Runtime error in ck_on_proc: {}", lua_tostring(g_L, -1));
		lua_pop(g_L, 1);

		ck_set_mod_context(previous_context);
		return false;
	}

	bool result = false;
	if (lua_isboolean(g_L, -1)) result = lua_toboolean(g_L, -1);

	lua_pop(g_L, 1);

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

	clear_lua_registry(g_clear_tracked_objects_ref, "state.clear_tracked_objects");
	clear_lua_registry(g_clear_registry_ref, "objects.clear_registry");

	ck_dispatcher_emit("onMapEnter");
}

// ffi

bool ck_dispatcher_load_mod(const char* mod_id) {
	log.info("mod_id: {}", mod_id);
	if (!g_L || g_load_and_init_mod_ref == LUA_NOREF || !mod_id) return false;

	std::string target_mod(mod_id);
	auto it = std::find(g_active_mods.begin(), g_active_mods.end(), target_mod);
	if (it == g_active_mods.end()) {
		g_active_mods.push_back(target_mod);
	}

	std::string previous_context = g_current_mod_id;
	ck_set_mod_context(mod_id);

	lua_rawgeti(g_L, LUA_REGISTRYINDEX, g_load_and_init_mod_ref);
	lua_pushstring(g_L, mod_id);

	if (lua_pcall(g_L, 1, LUA_MULTRET, 0) != LUA_OK) {
        log.error("Critical LuaJIT compilation error in mod '{}': {}", mod_id, lua_tostring(g_L, -1));
        lua_pop(g_L, 1);

        g_active_mods.erase(
            std::remove(g_active_mods.begin(), g_active_mods.end(), target_mod),
            g_active_mods.end()
        );

        ck_set_mod_context(previous_context.c_str());
        return false;
    }

	lua_pop(g_L, 1);

	ck_set_mod_context(previous_context.c_str());
	return true;
}

void ck_dispatcher_register_mod(const char* mod_id) {
	if (mod_id) g_active_mods.push_back(mod_id);
}

void ck_dispatcher_remove_mod(const char* mod_id) {
	if (!mod_id) return;

	std::string target_mod(mod_id);
	std::erase(g_active_mods, std::string(mod_id));

	log.info("Removed mod from dispatcher: {}", mod_id);
}

const char* ck_get_current_mod_id() {
	return g_current_mod_id;
}

void ck_dispatcher_emit_for_mod(const char* mod_id, const char* event_name) {
	if (!g_L || g_emit_for_mod_ref == LUA_NOREF || !mod_id || !event_name) return;

	std::string previous_context = g_current_mod_id;
	ck_set_mod_context(mod_id);

	invoke_lua_emit(mod_id, event_name);

	ck_set_mod_context(previous_context.c_str());
}

