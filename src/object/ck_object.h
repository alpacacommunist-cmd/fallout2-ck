// src/object/ck_object.h
#ifndef CK_SCRIPTING_OBJECT_H
#define CK_SCRIPTING_OBJECT_H

#include "object.h"

void ck_object_remove_blocker_at(int tile);
void ck_object_create_blocker_at(int tile);
void ck_object_create_at(int fid, int tile);
#endif
