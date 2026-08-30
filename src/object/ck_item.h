// src/object/ck_item.h
#ifndef CK_ITEM_H
#define CK_ITEM_H

#include "ck_api.h"
#include "object/ck_object.h"

namespace fallout {
	struct Object;
	struct UniqueObject;
	struct Rect;

    enum CritterFlags : int;

    bool critterFlagCheck(int pid, CritterFlags flag);

    int itemAdd(Object* owner, Object* itemToAdd, int quantity);
    int itemDropAll(Object* critter, int tile);

	int _obj_disconnect(Object* obj, Rect* rect);
	int objectDestroy(Object* object, Rect* rect);
	int objectCreateWithFidPid(UniqueObject& obj, int fid, int pid);
	int objectSetLocation(Object* obj, int tile, int elevation, Rect* rect);
	int objectGetCarriedQuantityByPid(Object* obj, int pid);
}

namespace ck {
	void clear_inventory(fallout::Object* object);
	bool inventory_add(void* container_ptr, int item_pid, int count, bool persistent = false);
}

CK_API bool ck_inventory_add(void* container_ptr, int item_pid, int count, bool persistent = false);
CK_API int  ck_inventory_count(void* container_ptr, int item_pid);
CK_API int  ck_total_inventory_count(fallout::Object *object);
CK_API bool ck_get_inventory_item(fallout::Object *object, int index, int *out_pid, int *out_qty);

#endif
