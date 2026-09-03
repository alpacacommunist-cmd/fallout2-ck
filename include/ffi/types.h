typedef struct fallout_Object fallout_Object;
typedef struct fallout_CritterProto fallout_CritterProto;

// basic 4-byte
typedef int Rotation;
typedef int ObjectFlags;
typedef int OutlineType;
typedef struct { uint8_t bytes[72]; } __attribute__((aligned(8))) ObjectData;

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
