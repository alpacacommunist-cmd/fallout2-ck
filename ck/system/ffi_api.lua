local ffi = require("ffi")

ffi.cdef[[
  // --- System & Bootstrap ---
  bool ck_dispatcher_load_mod(const char* mod_id);
  void ck_dispatcher_emit_for_mod(const char* mod_id, const char* event_name);

  void ck_registry_destroy_objects_for_mod(const char* target_mod_id);
  void ck_registry_clear();

  void ck_config_clear_mod_patches(const char* mod_id);

  const char* ck_get_current_mod_id();

  // --- Encoding ---
  size_t ck_cp1251_to_utf8(const char* in_cp1251, char* out_utf8, size_t max_size); size_t strlen(const char* s);

  void ck_scripting_load_game_slot(int slot);

  // --- Misc ---
  void ck_scripting_monitor_print_message(const char* message);
  void ck_sound_play_sfx(const char* name);

  // --- Tests ---
  const char* ck_testing_get_current_suite();
  void ck_testing_set_current_suite(const char* name);

  // --- Proto ---
  typedef struct {
    int pid;
    int fid;
    int type;
    int sid;

    const char* name;
    const char* filename;
    const char* description;
  } CkProtoInfoFFI;

  bool ck_proto_get_by_pid(int pid, CkProtoInfoFFI* out_info);
  bool ck_proto_get_by_name(const char* name, int type, CkProtoInfoFFI* out_info);
  int ck_proto_get_by_type(int type, CkProtoInfoFFI* out_array, int max_count);

  // --- State ---
  bool ck_state_load(const char* path);
  void ck_state_save(const char* path);

  // --- Combat & Actions ---
  bool ck_in_combat();

  // --- Object FFI ---
  typedef struct {
      void*     c_ptr;
      int       id;
      int       pid;
      int       sid;
      int       tile;
      int       elevation;
      int       flags;
      int       rotation;

      const char* name;

      int         lua_id;
      const char* mod_id;
  } CkObjectFFI;

  int ck_registry_modify_object(void* ptr);
  int ck_registry_restore_modified_object(void* ptr);

  int ck_object_find_at_tile(int tile, CkObjectFFI* buffer, int max_count);
  int ck_object_find_by_pid(int pid, CkObjectFFI* buffer, int max_count);

  // --- Critter Animations ---
  int  ck_anim_begin(void* ptr, int weapon_ready);
  int  ck_anim_move_to(void* ptr, int tile, int elevation);
  int  ck_anim_play(void* ptr, int anim_id);
  int  ck_anim_clear(void* ptr);
  int  ck_anim_end();
  bool ck_critter_is_busy(void* ptr);

  // --- Critter Stats & Base ---
  int  ck_critter_get_gender(void* ptr);
  int  ck_critter_get_base_stat(void* ptr, int stat_id);
  bool ck_critter_set_base_stat(void* ptr, int stat, int value);
  int  ck_critter_get_bonus_stat(void* ptr, int stat);
  bool ck_critter_set_bonus_stat(void* ptr, int stat, int value);
  int  player_stat(int stat);
  int  player_pc_stat(int stat);
  int  player_set_base_stat(int stat, int value);
  int  player_set_bonus_stat(int stat, int value);
  int  player_give_experience(int xp);

  typedef struct { int lua_id; const char* mod_id; } CritterLua;
  typedef struct { const char* name; const char* description; } CritterLuaProtoParams;

  CritterLua ck_critter_register(int pid, int tile, int elevation, const char* tag, const CritterLuaProtoParams* params);

  // --- Critter Events
  bool ck_critter_kill(int lua_id);
  bool ck_critter_process_turn(void* ptr, int lua_id);

  // --- Critter HP ---
  int  ck_critter_get_hp(void* ptr);
  int  ck_critter_get_max_hp(void* ptr);
  int  ck_critter_set_current_hp(void* ptr, int target_hp);
  int  ck_critter_set_full_hp(void* ptr);

  // --- Object Base ---
  void* ck_object_get_ptr(int lua_id);
  int ck_object_get_id(void* c_ptr);
  int ck_object_get_tile(void* c_ptr);
  int ck_object_get_sid(void* c_ptr);
  char* ck_object_get_name(void* ptr);
  int ck_object_get_type(void* ptr);
  bool ck_object_float_msg(void* ptr, const char* text, int msg_type);

  // --- Stats metadata ---
  void ck_get_stats_metadata(void (*callback)(const char* lua_name, int value));
  void ck_get_pc_stats_metadata(void (*callback)(const char* lua_name, int value));

  // --- Skills  ---
  void ck_get_skills_metadata(void (*callback)(const char* name, int value));

  int  player_skill(int skill);
  int  player_add_skill(int skill, int value);
  int  player_set_skill(int skill, int value);

  // --- Perks ---
  void ck_get_perks_metadata(void (*callback)(const char* name, int value));
  int player_perk(int perk_id);
  int player_perk_add_rank(int perk_id);

  // --- Items ---
  bool ck_inventory_add(void* container_ptr, int item_pid, int count);
  int  ck_inventory_count(void* container_ptr, int item_pid);

  // --- Rendering ---
  void ck_rendering_clear();
  void ck_rendering_refresh();

  // --- Assets ---
  int ck_assets_register_path(const char* path);
  int ck_ids_art_id_from_fid(int fid);

  // --- IDS ---
  int ck_ids_make_ck_fid(int custom_frm_id, int art_type);

  // --- Dialogue ---
  bool ck_dialog_init_ui();
  void ck_dialog_set_reply(const char* text);
  void ck_dialog_add_option(const char* text, int reaction);
  int  ck_dialog_go();
  void ck_dialog_exit();
  void ck_dialog_close_ui();

  // --- Game-time ---
  int ck_game_get_year();
  int ck_game_get_day();
  int ck_game_get_month();
  int ck_game_get_hour();
  int ck_game_get_time();

  // --- Locations ---

  typedef struct {
    const char* map_file;
    const char* name;
    const char* sub_name;
    const char* music;
    const char* sfx;
  } CkAreaMapFFI;

  int ck_area_register_map(const CkAreaMapFFI* data);
  int ck_area_register_location(const char* name, int world_x, int world_y, const char* size);
  int ck_area_expand_location(int area_id, const char* custom_map_lookup_name, int townmap_x, int townmap_y);

  // --- Map ---
  int  ck_map_get_mvar(int index);
  void ck_map_set_mvar(int index, int value);

  // --- remove object ---
  void ck_object_remove_at(int tile);
  bool ck_tile_is_blocked(int tile, int elevation);
  int ck_current_elevation();

  // --- add object/render
  int  ck_map_get_id();
  void ck_map_add_scenery_fid(int fid, int tile);
  void ck_map_add_tile_fid(int fid, int tile);
  void ck_map_create_blocker_at(int tile);
  void ck_map_create_object(int fid, int tile);
  void ck_map_create_object_fid(int fid, int tile);
  int  ck_map_register_object(int artId, int tile);


  typedef struct { int left; int right; int top; int bottom; } CkCameraBorders;
  void ck_map_set_camera_borders(int map_id, const CkCameraBorders* borders);
  void ck_map_clear_camera_borders_for_mod(const char* mod_id);

  // --- Map Batch ---
  typedef struct { int tile; int fid; } CkFFITile;
  typedef struct { int tile; int fid; } CkFFIScenery;
  typedef struct { int tile; int fid; } CkFFIBlocker;
  typedef struct { int tile; } CkFFIClear;

  void ck_map_batch_tiles(const CkFFITile* tiles, int count);
  void ck_map_batch_scenery(const CkFFIScenery* sceneries, int count);
  void ck_map_batch_blockers(const CkFFIBlocker* blockers, int count);
  void ck_map_batch_clear(const CkFFIClear* tiles, int count);

  // --- Exit Grid ---
  int ck_proto_first_exit_grid_pid();
  int ck_proto_last_exit_grid_pid();

  typedef struct {
    int target_map;
    int target_tile;
    int target_elevation;
    int target_rotation;
  } CKExitGridData;

  void ck_landscape_destroy_exit_grid_in_rect(int left, int right, int top, int bottom);
  void ck_landscape_create_exit_grid_in_rect(int t1, int t2, int t3, int t4, int pid, CKExitGridData data);
  void ck_landscape_create_exit_grid_at_tile(int tile, int pid, const CKExitGridData* data);

  // --- Custom Proto ---

  typedef struct {
    int weight; int price;

    int inv_fid;
    int ground_fid;

    bool usable;

    const char* name; const char* description;
  } CustomProtoFFI;

  typedef struct { int pid; const char* lua_tag; } CustomProtoLuaView;

  int ck_proto_register(int source_pid, int object_type, const char* lua_tag, const CustomProtoFFI* ffi_data);
  int ck_proto_get_pid_by_tag(const char* lua_tag);
  int ck_proto_bind(int pid);

  // --- Knowledge ---
  typedef struct {
    int id;
    const char* name;
    const char* description;
    int frm_id;
  } CkKnowledgeFFI;

  void ck_knowledge_clear_cache();
  void ck_knowledge_push_cache(const CkKnowledgeFFI* data);
]]

return ffi
