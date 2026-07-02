#include "ck_encoding.h"
#include "ck_ids.h"
#include "ck_lua_proxy/ck_lua_proxy_state.h"

#include "object/ck_object.h"
#include "object/ck_object_registry.h"
#include "object/critter/ck_critter.h"
#include "object/critter/ck_stats.h"

#include "ck_log.h"
static const Logger log("CK Critter");

namespace ck {

	int DEFAULT_CRITTER_SID = 13;

	fallout::Object* create_critter(int pid, int tile) {
		fallout::Object* critter = ck_object_create(pid, tile);

		if (critter != nullptr) {
			critter->sid    = ck::make_full_sid(fallout::SCRIPT_TYPE_CRITTER, DEFAULT_CRITTER_SID);

			return critter;
		}

		return nullptr;
	}

	CritterLua register_critter(int pid, int tile, const char* tag) {
		std::string mod_id  = ck_get_current_mod_id();
		std::string lua_tag = (tag != nullptr ? std::string(tag) : std::string());
		int map_id          = fallout::mapGetCurrentMap();
		ck::proxy::ObjectState state = ck::proxy::get_object_state(map_id, lua_tag);

		if (state.tile != -1) tile = state.tile;

		fallout::Object* critter = create_critter(pid, tile);
		if (critter == nullptr) return { -1, ck_get_current_mod_id() };

		if (state.hp != -1) ck::critter_adjust_hp(critter, state.hp);

		int lua_id = -1;
		LuaMeta meta = { critter->sid, lua_tag, mod_id };

		lua_id = gObjectRegistry.add(critter, meta);

		int custom_sid = ck::make_sid(lua_id);
		critter->sid   = ck::make_full_sid(fallout::SCRIPT_TYPE_CRITTER, custom_sid);

		return { lua_id, ck_get_current_mod_id() };
	}

}

void ck_critter_float_msg(int lua_id, const char* text, int msg_type) {
	const CkManagedObject* managed = gObjectRegistry.get_managed(lua_id);
	if (!managed || !managed->ptr) return;

	fallout::Object* obj = managed->ptr;
	if (obj->elevation != fallout::gElevation) return;

	int color = fallout::_colorTable[32747], background_color = fallout::_colorTable[0], font = 101;

	switch (msg_type) {
		case 1: // (FLOATING_MESSAGE_TYPE_WHITE)
			color = fallout::_colorTable[32767];
			break;
		case 2: // (FLOATING_MESSAGE_TYPE_RED)
			color = fallout::_colorTable[31744];
			break;
		case 3: // (FLOATING_MESSAGE_TYPE_GREEN)
			color = fallout::_colorTable[992];
			break;
		case 4: // (FLOATING_MESSAGE_TYPE_BLUE)
			color = fallout::_colorTable[31];
			break;
		default: // YELLOW
			color = fallout::_colorTable[32747];
			break;
	}

	fallout::Rect rect;
	std::string converted = utf8_to_cp1251(text);

	if (fallout::textObjectAdd(obj, const_cast<char*>(converted.c_str()), font, color, background_color, &rect) != -1) {
		fallout::tileWindowRefreshRect(&rect, obj->elevation);
	}
}

CritterLua ck_critter_register(int pid, int tile, const char* tag) {
	return ck::register_critter(pid, tile, tag);
}

int ck_anim_begin(void* ptr, int request_options) {
	return fallout::reg_anim_begin(request_options);
}

int ck_anim_move_to(void* ptr, int tile, int elevation) {
	if (!ptr) return -1; auto* object = static_cast<fallout::Object*>(ptr);

	return fallout::animationRegisterMoveToTile(object, tile, elevation, -1, 0);
}

int ck_anim_play(void* ptr, int anim_id) {
	if (!ptr) return -1; auto* object = static_cast<fallout::Object*>(ptr);

	return fallout::animationRegisterAnimate(object, anim_id, 0);
}

int ck_anim_clear(void* ptr) {
	if (!ptr) return -1; auto* object = static_cast<fallout::Object*>(ptr);
	return fallout::reg_anim_clear(object);
}

int ck_anim_end() {
	return fallout::reg_anim_end();
}

bool ck_critter_is_busy(void* ptr) {
	if (!ptr) return false; auto* obj = static_cast<fallout::Object*>(ptr);

	return fallout::animationIsBusy(obj) == -1;
}

