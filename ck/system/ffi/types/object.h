// LUA HEADER

typedef struct fallout_Object fallout_Object;
typedef struct fallout_CritterProto fallout_CritterProto;

// basic 4-byte
typedef enum {
    ROTATION_INVALID = -1,
    ROTATION_NE = 0,
    ROTATION_E = 1,
    ROTATION_SE = 2,
    ROTATION_SW = 3,
    ROTATION_W = 4,
    ROTATION_NW = 5,
    ROTATION_COUNT = 6,
    ROTATION_FIRST = ROTATION_NE,
    ROTATION_LAST = ROTATION_NW,
} Rotation;

typedef int ObjectFlags;
typedef int OutlineType;
typedef int CritterManeuver;
typedef int Dam;
typedef int Map;
typedef int MapElevation;

// ObjectData
typedef struct InventoryItem {
    fallout_Object* item;
    int quantity;
} InventoryItem;

typedef struct Inventory {
    int length;
    int capacity;
    InventoryItem* items;
} Inventory;

typedef struct WeaponObjectData {
    int ammoQuantity;
    int ammoTypePid;
} WeaponObjectData;

typedef struct AmmoItemData {
    int quantity;
} AmmoItemData;

typedef struct MiscItemData {
    int charges;
} MiscItemData;

typedef struct KeyItemData {
    int keyCode;
} KeyItemData;

typedef union ItemObjectData {
    WeaponObjectData weapon;
    AmmoItemData ammo;
    MiscItemData misc;
    KeyItemData key;
} ItemObjectData;

typedef struct CritterCombatData {
    CritterManeuver maneuver;
    int ap;
    Dam results;
    int damageLastTurn;
    int aiPacket;
    int team;
    union {
        fallout_Object* whoHitMe;
        int whoHitMeCid;
    };
} CritterCombatData;

typedef struct CritterObjectData {
    int reaction;
    CritterCombatData combat;
    int hp;
    int radiation;
    int poison;
} CritterObjectData;

typedef struct DoorSceneryData {
    int openFlags;
} DoorSceneryData;

typedef struct StairsSceneryData {
    Map destinationMap;
    int destinationBuiltTile;
} StairsSceneryData;

typedef struct ElevatorSceneryData {
    int type;
    int level;
} ElevatorSceneryData;

typedef struct LadderSceneryData {
    Map destinationMap;
    int destinationBuiltTile;
} LadderSceneryData;

typedef union SceneryObjectData {
    DoorSceneryData door;
    StairsSceneryData stairs;
    ElevatorSceneryData elevator;
    LadderSceneryData ladder;
} SceneryObjectData;

typedef struct MiscObjectData {
    Map map;
    int tile;
    MapElevation elevation;
    Rotation rotation;
} MiscObjectData;

typedef struct ObjectData {
    Inventory inventory;
    union {
        CritterObjectData critter;
        struct {
            int flags;
            union {
                ItemObjectData item;
                SceneryObjectData scenery;
                MiscObjectData misc;
            };
        };
    };
} ObjectData;

// fallout2-ce obj_types.h
struct fallout_Object {
    int id; // obj_id
    int tile; // obj_tile_num
    int x; // obj_x
    int y; // obj_y
    int sx; // obj_sx
    int sy; // obj_sy
    int frame; // obj_cur_frm
    Rotation rotation; // obj_cur_rot
    int fid; // obj_fid
    ObjectFlags flags; // obj_flags
    int elevation; // obj_elev
    ObjectData data;
    int pid; // obj_pid
    int cid; // obj_cid
    int lightDistance; // obj_light_distance
    int lightIntensity; // obj_light_intensity
    OutlineType outline; // obj_outline
    int sid; // obj_sid
    fallout_Object* owner;
    int scriptIndex; // TODO: remove
};
