#include "ffi/system.h"
#include "ffi/tests.h"
#include "ffi/gameplay.h"

// TODO:: move to headers
extern "C" {
    int ck_item_proto_register(...);
    int ck_assets_register_path(...);
    bool ck_proto_get_by_pid(...);
	int ck_inventory_count(...);
    int player_stat(...);
    void ck_get_perks_metadata(...);
	void ck_get_skills_metadata(...);
    void ck_map_batch_tiles(...);
}

namespace ck::ffi {
    void init_ffi_manifest() {
        static void* const volatile force_linker[] = {
            // system.h
            (void*)&ck_system_add_mod,
            // ...

            // tests.h
            (void*)&ck_testing_get_current_suite,
            // ...

            // gameplay.h
            (void*)&ck_monitor_print_message,
            // ...

            // TODO:: move to headers
            (void*)&ck_item_proto_register,
            (void*)&ck_assets_register_path,
            (void*)&ck_proto_get_by_pid,
            (void*)&ck_inventory_count,
            (void*)&player_stat,
            (void*)&ck_object_float_msg,
            (void*)&ck_get_perks_metadata,
            (void*)&ck_get_skills_metadata,
            (void*)&ck_map_batch_tiles,
        };
        (void)force_linker;
    }
}

