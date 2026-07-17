#include "ce_config/ck_config_patch.h"

#include "ck_log.h"
static const Logger log("CK Party");

namespace ck {
	bool party_patch_config(const CkPartyConfigFFI* party_config) {
        int assigned_index = ck::config_party::register_companion(party_config);

        return assigned_index != -1;
    }
}
