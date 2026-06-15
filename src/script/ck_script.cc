#include "ck_ids.h"
#include "ck_utils.h"

#include "scripts.h"
#include "display_monitor.h"
#include "object/ck_object_registry.h"

#include <iostream>
#include <string>

namespace ck {
	bool script_try_handle(int sid, int proc) {
		if (is_ck_sid(clean_sid(sid))) {
			int lua_id = lua_id_from_sid(clean_sid(sid));
			const LuaCritterMeta* meta = gObjectRegistry.get_meta(lua_id);

			std::cout << "[CK] got script handle for : " << lua_id  << std::endl;

			if (proc == fallout::SCRIPT_PROC_LOOK_AT) {
				fallout::displayMonitorAddMessage(meta->name.c_str());
			}

			if (proc == fallout::SCRIPT_PROC_DESCRIPTION) {
				fallout::displayMonitorAddMessage(meta->description.c_str());
			}

			return true;
		}

		return false;
	}

} // namespace ck
