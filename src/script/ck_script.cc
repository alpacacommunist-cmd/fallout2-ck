#include "ck_ids.h"
#include "ck_utils.h"

#include "scripts.h"
#include "display_monitor.h"
#include "object/ck_object_registry.h"

#include <iostream>
#include <string>

namespace ck {

	static fallout::Script gDummyScript;

	// void script_init() {
	// 	std::fill(reinterpret_cast<char*>(&gDummyScript),
	// 			reinterpret_cast<char*>(&gDummyScript) + sizeof(fallout::Script), 0);
	//
	// 	gDummyScript.sp.built_tile = -1;
	// 	gDummyScript.sp.radius = -1;
	// 	gDummyScript.actionBeingUsed = -1;
	//
	// 	for (int index = 0; index < fallout::SCRIPT_PROC_COUNT; index++) {
	// 		gDummyScript.procs[index] = fallout::SCRIPT_PROC_NO_PROC;
	// 	}
	// }

	bool owns_sid(int sid) {
		return ck::is_ck_sid(ck::clean_sid(sid));
	}

	fallout::Script* script_get_dummy(int sid) {
		gDummyScript.sid = sid;
		return &gDummyScript;
	}

	bool script_try_handle(int sid, int proc) {
		if (!is_ck_sid(clean_sid(sid))) return false;

		int lua_id = lua_id_from_sid(clean_sid(sid));
		const CkManagedObject* managed = gObjectRegistry.get_managed(lua_id);

		if (!managed) return false;

		gDummyScript.scriptOverrides = 0;

		switch (proc) {
			case fallout::SCRIPT_PROC_LOOK_AT:
				if (!managed->meta.name.empty())
					fallout::displayMonitorAddMessage(managed->meta.name.c_str());

				gDummyScript.scriptOverrides = 1;
				return true;

			case fallout::SCRIPT_PROC_DESCRIPTION:
				if (!managed->meta.description.empty()) {
					fallout::displayMonitorAddMessage(managed->meta.description.c_str());
				}

				gDummyScript.scriptOverrides = 1;
				return true;

			case fallout::SCRIPT_PROC_TALK:
				return true;

			default:
				if (managed->meta.proto_sid != -1 && managed->ptr) {
					int saved         = managed->ptr->sid;
					managed->ptr->sid = managed->meta.proto_sid;
					fallout::scriptExecProc(managed->meta.proto_sid, proc);
					managed->ptr->sid = saved;

					return true;
				}

				return false;
		}
	}

} // namespace ck
