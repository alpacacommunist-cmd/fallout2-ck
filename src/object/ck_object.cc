#include "ck_scripting.h"
#include "ck_object.h"

void ck_object_destroy(fallout::Object* blocker) {
	fallout::Rect rect;
	fallout::objectGetRect(blocker, &rect);

	fallout::objectDestroy(blocker, &rect);
}

