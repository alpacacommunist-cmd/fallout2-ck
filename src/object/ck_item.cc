#include "utils.h"
#include "utils/fallout/types.h"

#include "object/ck_item.h"
#include "ck_proto/registry/ck_proto_registry.h"

#include "fallout2-ce/item.h"

#include "log.h"
static constexpr logging::Context logger{"ck_item.cc"};

namespace fallout {
    bool critterFlagCheck(const ProtoId& protoId, CritterFlags flag);
    int protoGetProto(const ProtoId &protoId, Proto **protoPtr);
}

namespace ck {
	void clear_inventory(fallout::Object* object) {
		fallout::Inventory* inventory = &(object->data.inventory);

        if (inventory && inventory->items) {
            for (int i = inventory->length - 1; i >= 0; i--) {
                fallout::Object *item = inventory->items[i].item;
                int qty = inventory->items[i].quantity;

                if (item) {
                    fallout::itemRemove(object, item, qty);
                    fallout::objectDestroy(item, nullptr);
                }
            }
        }
    }

	bool inventory_add(void* container_ptr, int item_pid, int count) {
		if (!container_ptr || count < 1) return false;
		auto* owner = static_cast<fallout::Object*>(container_ptr);

        fallout::Proto *proto = nullptr;
        fallout::ProtoId item_proto_id(item_pid);

        if (fallout::protoGetProto(item_proto_id, &proto) == -1 || !proto) {
			return false;
		}

		fallout::Object* new_item = nullptr;
		if (fallout::objectCreateWithProtoId(&new_item, item_proto_id) == -1 || !new_item) {
			return false;
		}

		fallout::objectSetLocation(new_item, 0, f_types::map_elevation(fallout::gElevation), nullptr);

		if (fallout::itemAdd(owner, new_item, count) != 0) {
			fallout::objectDestroy(new_item, nullptr);
			return false;
		}

        // Proto* custom_proto = ck::proto::registry::get(item_pid);
        // if (custom_proto) {
        //     new_item->sid = custom_proto->get_sid();
        // }

		fallout::_obj_disconnect(new_item, nullptr);

        if (fallout::critterFlagCheck(fallout::ProtoId(owner->pid), fallout::CritterFlags(fallout::CRITTER_NO_STEAL))) {
            logger.info("DROPPING ITEMS ON THE GROUND");
            fallout::itemDropAll(owner, owner->tile);
        }

		return true;
	}
}

int ck_inventory_count(void* container_ptr, int item_pid) {
	if (!container_ptr) return 0;
	auto* owner = static_cast<fallout::Object*>(container_ptr);

	return fallout::objectGetCarriedQuantityByProtoId(owner, fallout::ProtoId(item_pid));
}

bool ck_inventory_add(void* container_ptr, int item_pid, int count) {
	return ck::inventory_add(container_ptr, item_pid, count);
}

int ck_total_inventory_count(fallout::Object *object) {
    CK_ENSURE_VALID_OBJECT(object);
    return object->data.inventory.length;
}

bool ck_get_inventory_item(fallout::Object *object, int index, int *out_pid, int *out_qty) {
    CK_ENSURE_VALID_OBJECT(object);
    fallout::Inventory *inv = &(object->data.inventory);
    if (!inv || !inv->items || index < 0 || index >= inv->length) return false;

    fallout::Object *item = inv->items[index].item;
    if (!item) return false;

    *out_pid = item->pid;
    *out_qty = inv->items[index].quantity;
    return true;
}
