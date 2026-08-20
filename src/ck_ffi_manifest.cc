extern "C" {
    int ck_proto_register(...);
    int ck_assets_register_path(...);
    bool ck_proto_get_by_pid(...);
	int ck_inventory_count(...);
    int player_stat(...);
    void ck_object_float_msg(...);
    void ck_get_perks_metadata(...);
	void ck_get_skills_metadata(...);
    void ck_map_batch_tiles(...);
}

static void* const volatile ck_ffi_exports[] = {
    (void*)&ck_proto_register,
    (void*)&ck_assets_register_path,
    (void*)&ck_proto_get_by_pid,
    (void*)&ck_inventory_count,
    (void*)&player_stat,
    (void*)&ck_object_float_msg,
    (void*)&ck_get_perks_metadata,
    (void*)&ck_get_skills_metadata,
    (void*) ck_map_batch_tiles
};

namespace ck {
    void init_ffi_manifest() {
        (void)ck_ffi_exports;
    }
}
