// src/object/ck_item.h
#ifndef CK_ITEM_H
#define CK_ITEM_H

#include "ck_api.h"
#include "proto.h"
#include "object/ck_object.h"

namespace fallout {
    int itemAdd(Object* owner, Object* itemToAdd, int quantity);
}

namespace ck {
	void clear_inventory(fallout::Object* object);
}

CK_API bool ck_inventory_add(void* container_ptr, int item_pid, int count);
CK_API int ck_inventory_count(void* container_ptr, int item_pid);

#endif
