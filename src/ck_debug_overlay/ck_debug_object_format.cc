#include "ck_debug_overlay/ck_debug_object_format.h"

#include "obj_types.h"
#include <format>

#include "ck_log.h"
static const Logger log("CK DBG format");

namespace ck::debug {
	void export_full_dump(const std::vector<ckDebugHex*>& hexes) {
		static int grid_width = fallout::tileGetHexGridWidth();

		for (ckDebugHex* hex : hexes) {
			int tile = hex->tile;
			int tileX = grid_width - 1 - tile % grid_width;
			int tileY = tile / grid_width;

			log.raw("SELECTED tile={} ({}, {})", tile, tileX, tileY);

			if (fallout::isExitGridAt(tile, fallout::gElevation)) {
				log.warn("EXIT GRID ON TILE");
			}

			fallout::Object* obj = fallout::objectFindFirstAtLocation(fallout::gElevation, tile);
			int objIndex = 0;

			while (obj != nullptr) {
				int objType = FID_TYPE(obj->fid);

				log.raw("[OBJ #{} Name: {}, ID: {} | Type: {}, PID: {}, FID: {}, SID: {}, Flags: {:#x}",
						objIndex, fallout::objectGetName(obj), obj->id, objType, obj->pid, obj->fid, obj->sid,
						static_cast<unsigned int>(obj->flags));

				std::string data_debug = ck::debug::format_object_data(obj, objType);
				log.raw("{}", data_debug);

				obj = fallout::objectFindNextAtLocation();
			}
		}
	}

    void export_lua_tiles(const std::vector<ckDebugHex*>& hexes) {
        log.raw("tiles = {{");
        for (ckDebugHex* hex : hexes) {
            if (hex->state == HexState::SELECTED) {
                log.raw("  {{ tile = {} }},", hex->tile);
            }
        }
        log.raw("}}");
    }

    std::string format_object_data(fallout::Object* obj, int obj_type) {
        if (obj == nullptr) return "  data: NULL";

        const auto& d = obj->data;

        switch (obj_type) {
            // case 0: // OBJ_TYPE_ITEM
            //     return std::format(
            //         "  [Item Data] Count: {}, Flags: {:#x}, PID: {}",
            //         d.item.count,
            //         d.flags,
            //         d.item.pid
            //     );

            case 1: // OBJ_TYPE_CRITTER
                return std::format(
                    "  [Critter Data] HP: {}/{}, Rad: {}, Poison: {} | "
                    "AI_Packet: {}, Team: {}, AP: {}, Target: {}",
                    d.critter.hp, ck_critter_get_max_hp(obj),
                    d.critter.radiation,
                    d.critter.poison,
                    d.critter.combat.aiPacket,
                    d.critter.combat.team,
                    d.critter.combat.ap,
                    d.critter.combat.whoHitMe ? "YES" : "NONE"
                );

			case 2: // OBJ_TYPE_SCENERY
				return std::format(
					"  [Scenery Data] Int1(Flags/DestMap/ElevType): {} | "
					"Int2(DestTile/ElevLevel): {}",
					d.scenery.door.openFlags,
					d.scenery.elevator.level
				);

            case 3: // OBJ_TYPE_WALL
                return std::format("  [Wall Data] Flags: {:#x}", d.flags);

            case 4: // OBJ_TYPE_TILE
                return std::format("  [Tile Data] Flags: {:#x}", d.flags);

            case 5: // OBJ_TYPE_MISC
                return std::format(
                    "  [Misc Data] Map: {}, Tile: {}, Elev: {}, Rot: {}, Flags: {:#x}",
                    d.misc.map,
                    d.misc.tile,
                    d.misc.elevation,
                    d.misc.rotation,
                    d.flags
                );

            default:
                return "  [Unknown Type Data]";
        }
    }

} // namespace ck::debug
