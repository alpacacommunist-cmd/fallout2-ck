#ifndef CK_CONFIG_PARTY_H
#define CK_CONFIG_PARTY_H

#include <string>
#include "/party/ck_party.h"

namespace ck::config_party {
    int get_next_index();

    std::string format_section(int member_idx);

    int register_companion(const CkPartyConfigFFI* party_config);
}

#endif
