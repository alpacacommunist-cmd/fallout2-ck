extern "C" {
    int player_stat(...);

    void ck_critter_float_msg(...);

	void ck_get_skills_metadata(...);
}

static void* const volatile ck_ffi_exports[] = {
    (void*)&player_stat,

    (void*)&ck_critter_float_msg,

    (void*)&ck_get_skills_metadata
};

namespace ck {
    void init_ffi_manifest() {
        (void)ck_ffi_exports;
    }
}
