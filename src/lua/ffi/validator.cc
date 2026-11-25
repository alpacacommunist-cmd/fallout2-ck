#include <cstddef>
#include "fallout2-ce/obj_types.h"
#include "fallout2-ce/animation_defs.h"

void validate_object_data_structure() {
    using namespace fallout;

    static_assert(sizeof(InventoryItem) == 16, "🚨 InventoryItem size mismatch!");
    static_assert(sizeof(Inventory) == 16, "🚨 Inventory size mismatch!");
    static_assert(sizeof(ItemObjectData) == 8, "🚨 ItemObjectData size mismatch!");
    static_assert(sizeof(SceneryObjectData) == 8, "🚨 SceneryObjectData size mismatch!");
    static_assert(sizeof(MiscObjectData) == 16, "🚨 MiscObjectData size mismatch!");
    static_assert(sizeof(CritterCombatData) == 32, "🚨 CritterCombatData size mismatch!");
    static_assert(sizeof(CritterObjectData) == 56, "🚨 CritterObjectData size mismatch!");
    static_assert(sizeof(ObjectData) == 72, "🚨 ObjectData size mismatch!");

    static_assert(offsetof(ObjectData, inventory) == 0, "🚨 ObjectData.inventory offset mismatch!");
    static_assert(offsetof(ObjectData, critter) == 16, "🚨 ObjectData.critter offset mismatch!");
    static_assert(offsetof(ObjectData, flags) == 16, "🚨 ObjectData.flags offset mismatch!");
    static_assert(offsetof(ObjectData, item) == 20, "🚨 ObjectData.item offset mismatch!");
    static_assert(offsetof(ObjectData, scenery) == 20, "🚨 ObjectData.scenery offset mismatch!");
    static_assert(offsetof(ObjectData, misc) == 20, "🚨 ObjectData.misc offset mismatch!");

    static_assert(offsetof(CritterObjectData, combat) == 8, "🚨 CritterObjectData.combat offset mismatch!");
    static_assert(offsetof(CritterObjectData, hp) == 40, "🚨 CritterObjectData.hp offset mismatch!");
    static_assert(offsetof(CritterObjectData, radiation) == 44, "🚨 CritterObjectData.radiation offset mismatch!");
    static_assert(offsetof(CritterObjectData, poison) == 48, "🚨 CritterObjectData.poison offset mismatch!");

    static_assert(sizeof(WeaponObjectData) == 8, "🚨 WeaponObjectData size mismatch!");
    static_assert(sizeof(AmmoItemData) == 4, "🚨 AmmoItemData size mismatch!");
    static_assert(sizeof(MiscItemData) == 4, "🚨 MiscItemData size mismatch!");
    static_assert(sizeof(KeyItemData) == 4, "🚨 KeyItemData size mismatch!");
}

void validate_rotation_enum() {
    using namespace fallout;

    static_assert(sizeof(Rotation) == 4, "🚨 Rotation size mismatch!");
    static_assert(offsetof(Object, rotation) == 28, "🚨 Object.rotation offset mismatch!");

    static_assert(ROTATION_INVALID == -1, "🚨 ROTATION_INVALID mismatch!");
    static_assert(ROTATION_NE == 0, "🚨 ROTATION_NE mismatch!");
    static_assert(ROTATION_E == 1, "🚨 ROTATION_E mismatch!");
    static_assert(ROTATION_SE == 2, "🚨 ROTATION_SE mismatch!");
    static_assert(ROTATION_SW == 3, "🚨 ROTATION_SW mismatch!");
    static_assert(ROTATION_W == 4, "🚨 ROTATION_W mismatch!");
    static_assert(ROTATION_NW == 5, "🚨 ROTATION_NW mismatch!");
    static_assert(ROTATION_COUNT == 6, "🚨 ROTATION_COUNT mismatch!");
}

void validate_object_structure() {
    using namespace fallout;
    static_assert(sizeof(Object) == 160, "🚨 fallout::Object size changed, refresh lua header!");

    static_assert(offsetof(Object, data) == 48, "🚨 fields.data offsets missmatch!");
    static_assert(offsetof(Object, pid) == 120, "🚨 fields.pid offsets missmatch!");
    static_assert(offsetof(Object, owner) == 144, "🚨 fields.owner offsets missmatch!");
}

void validate_animation_enums() {
    using namespace fallout;

    static_assert(ANIMATION_REQUEST_NONE == 0x00, "🚨 ANIMATION_REQUEST_NONE mismatch!");
    static_assert(ANIMATION_REQUEST_UNRESERVED == 0x01, "🚨 ANIMATION_REQUEST_UNRESERVED mismatch!");
    static_assert(ANIMATION_REQUEST_RESERVED == 0x02, "🚨 ANIMATION_REQUEST_RESERVED mismatch!");
    static_assert(ANIMATION_REQUEST_NO_STAND == 0x04, "🚨 ANIMATION_REQUEST_NO_STAND mismatch!");
    static_assert(ANIMATION_REQUEST_PING == 0x100, "🚨 ANIMATION_REQUEST_PING mismatch!");
    static_assert(ANIMATION_REQUEST_INSIGNIFICANT == 0x200, "🚨 ANIMATION_REQUEST_INSIGNIFICANT mismatch!");
}
