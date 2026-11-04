#include "utils/fallout/types.h"

#include "object/ck_object.h"
#include "ck_registry/ck_registry.h"
#include "geometry/geometry.h"
#include "geometry/landscape.h"
#include "ck_mods/ck_mods.h"

#include "fallout2-ce/proto_types.h"

#include "log.h"
static constexpr logging::Context logger{"geometry/landscape.cc"};

namespace f_types = ck::utils::types;
namespace fallout {
    struct Object;
    extern Object* gEgg;
    extern Object* gDude;

	extern int gElevation;

	int tileGetRotationTo(int tile1, int tile2);
	int tileGetTileInDirection(int tile, int rotation, int distance);

	Object* objectFindFirstAtLocation(fallout::MapElevation elevation, int tile);
	Object* objectFindNextAtLocation();
}

void ck_landscape_toggle_visibility_in_rect(const HexRect& rect, bool visible) {
    rect.for_each_tile([visible](int tile) {
        fallout::Object* object = fallout::objectFindFirstAtLocation(f_types::map_elevation(fallout::gElevation), tile);

        while (object != nullptr) {
            fallout::Object* next_object = fallout::objectFindNextAtLocation();

            if (fallout::ProtoId(object->pid).objectType() != fallout::OBJ_TYPE_CRITTER && object != fallout::gEgg) {
                if (visible) ck::registry::deleted::remove(object);
                else ck::registry::deleted::add(object);
            }

            object = next_object;
        }
	});
}

// FFI
int ck_proto_first_exit_grid_pid()  { return fallout::ProtoId(fallout::MiscProtoTypeId::FirstExitGrid).pid(); }
int ck_proto_last_exit_grid_pid()   { return fallout::ProtoId(fallout::MiscProtoTypeId::LastExitGrid).pid(); }

void ck_landscape_destroy_exit_grid_in_rect(int left, int right, int top, int bottom) {
    std::vector<int> tiles = { left, right, top, bottom };
    HexRect rect = geometry_build_rect_from_points(tiles);
    if (!rect.is_valid()) return;

    rect.for_each_tile([](int tile) {
        fallout::Object* obj = fallout::objectFindFirstAtLocation(f_types::map_elevation(fallout::gElevation), tile);
        while (obj != nullptr) {
            fallout::Object* next_obj = fallout::objectFindNextAtLocation();

            if (obj->pid >= ck_proto_first_exit_grid_pid() && obj->pid <= ck_proto_last_exit_grid_pid()) {
				ck::registry::deleted::add(obj);
            }
            obj = next_obj;
        }
    });
}

void ck_landscape_create_exit_grid_in_rect(int t1, int t2, int t3, int t4, int pid, CKExitGridData data) {
	std::vector<int> tiles = { t1, t2, t3, t4 };
	HexRect rect = geometry_build_rect_from_points(tiles);

	if (!rect.is_valid()) return;

	rect.for_each_tile([pid, &data](int tile) {
        fallout::Object* object = ck_object_create(pid, tile);
        if (object == nullptr) return;

        int object_id = ck::registry::created::add(object, nullptr, ck::mods::current_mod_id());

		if (object_id != -1) {
			object->data.misc.map       = fallout::Map(data.target_map);
			object->data.misc.tile      = data.target_tile;
			object->data.misc.elevation = fallout::MapElevation(data.target_elevation);
			object->data.misc.rotation  = static_cast<fallout::Rotation>(data.target_rotation);
		} else {
            logger.error("Couldn't created registry entry for created object: {}", object->pid);
        }
	});
}

void ck_landscape_create_exit_grid_at_tile(int tile, int pid, const CKExitGridData* data) {
    if (data == nullptr) return;

    fallout::Object* object = ck_object_create(pid, tile);
    if (object == nullptr) return;

    int object_id = ck::registry::created::add(object, nullptr, ck::mods::current_mod_id());

    if (object_id != -1) {
        object->data.misc.map       = fallout::Map(data->target_map);
        object->data.misc.tile      = data->target_tile;
        object->data.misc.elevation = fallout::MapElevation(data->target_elevation);
        object->data.misc.rotation  = static_cast<fallout::Rotation>(data->target_rotation);
    }
}
