// src/object/ck_item.h
#ifndef CK_ITEM_H
#define CK_ITEM_H

#include "ck_api.h"
#include "object/ck_object.h"

namespace fallout {
	struct Object;
	struct UniqueObject;
	struct Rect;

    int itemAdd(Object* owner, Object* itemToAdd, int quantity);
	int _obj_disconnect(Object* obj, Rect* rect);
	int objectDestroy(Object* object, Rect* rect);
	int objectCreateWithFidPid(UniqueObject& obj, int fid, int pid);
	int objectSetLocation(Object* obj, int tile, int elevation, Rect* rect);

	int objectGetCarriedQuantityByPid(Object* obj, int pid);
}

namespace ck {
	void clear_inventory(fallout::Object* object);
	bool inventory_add(void* container_ptr, int item_pid, int count);
}

CK_API bool ck_inventory_add(void* container_ptr, int item_pid, int count);
CK_API int ck_inventory_count(void* container_ptr, int item_pid);

#endif
