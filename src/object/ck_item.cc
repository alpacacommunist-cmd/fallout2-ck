#include "object/ck_item.h"
#include "ck_proto/ck_proto_registry.h"

#include "item.h"
#include "object.h"
#include "proto.h"

#include "ck_log.h"
static const Logger logger("CK Item");

namespace ck {
	void clear_inventory(fallout::Object* object) {
		fallout::Inventory* inventory = &(object->data.inventory);

		if (inventory && inventory->items) {
			for (int i = inventory->length - 1; i >= 0; i--) {
				fallout::Object* item = inventory->items[i].item;
				int               qty = inventory->items[i].quantity;

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

		fallout::Proto* proto = nullptr;
		if (fallout::protoGetProto(item_pid, &proto) == -1 || !proto) {
			return false;
		}

		fallout::Object* new_item = nullptr;
		if (fallout::objectCreateWithFidPid(&new_item, proto->fid, item_pid) == -1 || !new_item) {
			return false;
		}

		fallout::objectSetLocation(new_item, 0, 0, nullptr);

		if (fallout::itemAdd(owner, new_item, count) != 0) {
			fallout::objectDestroy(new_item, nullptr);
			return false;
		}

		fallout::_obj_disconnect(new_item, nullptr);

        if (fallout::critterFlagCheck(owner->pid, fallout::CritterFlags(fallout::CRITTER_NO_STEAL))) {
            logger.info("DROPPING ITEMS ON THE GROUND");
            fallout::itemDropAll(owner, owner->tile);
        }

		return true;
	}
}

int ck_inventory_count(void* container_ptr, int item_pid) {
	if (!container_ptr) return 0;
	auto* owner = static_cast<fallout::Object*>(container_ptr);

	return fallout::objectGetCarriedQuantityByPid(owner, item_pid);
}

bool ck_inventory_add(void* container_ptr, int item_pid, int count) {
	return ck::inventory_add(container_ptr, item_pid, count);
}

int ck_total_inventory_count(fallout::Object *object) {
    if (!object) return 0;
    return object->data.inventory.length;
}

bool ck_get_inventory_item(fallout::Object *object, int index, int *out_pid, int *out_qty) {
    if (!object) return false;
    fallout::Inventory *inv = &(object->data.inventory);
    if (!inv || !inv->items || index < 0 || index >= inv->length) return false;

    fallout::Object *item = inv->items[index].item;
    if (!item) return false;

    *out_pid = item->pid;
    *out_qty = inv->items[index].quantity;
    return true;
}
