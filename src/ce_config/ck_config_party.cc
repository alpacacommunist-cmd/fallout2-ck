#include "ce_config/ck_config_party.h"
#include "ce_config/ck_config_patch.h"
#include <format>
#include "ck_log.h"

static const Logger log("CK Party Config");

namespace ck::config_party {

    int get_next_index() {
        return ck::config_find_next_free_index_vfs("data\\data\\party.txt", "Party Member");
    }

    std::string format_section(int member_idx) {
        return std::format("Party Member {}", member_idx);
    }

    int register_companion(const CkPartyConfigFFI* party_config) {
        if (!party_config) return -1;

        static int next_member_index = -1;
        if (next_member_index == -1) {
            next_member_index = get_next_index();
        }

        std::string party_txt_path = "data/party.txt";
        int member_idx = -1;

		// mod reload check
        auto file_it = g_config_patches.find(normalize_config_path(party_txt_path));
        if (file_it != g_config_patches.end()) {
            for (const auto& [section, keys] : file_it->second) {
                auto pid_it = keys.find("party_member_pid");

                if (pid_it != keys.end() && pid_it->second == std::to_string(party_config->unique_pid)) {
                    // get index from section name: "Party Member X"
                    member_idx = std::stoi(section.substr(13));
                    log.info("Companion reload detected for PID {}: retaining existing Member ID {}",
                             party_config->unique_pid, member_idx);
                    break;
                }
            }
        }

        if (member_idx == -1) {
            member_idx = next_member_index++;
        }

        std::string section_name = format_section(member_idx);

        log.info("Registering party member patches: [{}] for PID: {}", section_name, party_config->unique_pid);

        ck::config_patch_add(party_txt_path, section_name, "party_member_pid", std::to_string(party_config->unique_pid));
        ck::config_patch_add(party_txt_path, section_name, "area_attack_mode", party_config->area_attack_mode);
        ck::config_patch_add(party_txt_path, section_name, "attack_who",       party_config->attack_who);
        ck::config_patch_add(party_txt_path, section_name, "best_weapon",      party_config->best_weapon);
        ck::config_patch_add(party_txt_path, section_name, "chem_use",         "clean");
        ck::config_patch_add(party_txt_path, section_name, "distance",         party_config->distance);
        ck::config_patch_add(party_txt_path, section_name, "run_away_mode",    "none");
        ck::config_patch_add(party_txt_path, section_name, "disposition",      "none");
        ck::config_patch_add(party_txt_path, section_name, "level_minimum",    "1");
        ck::config_patch_add(party_txt_path, section_name, "level_up_every",   "1");
        ck::config_patch_add(party_txt_path, section_name, "level_pids",       std::to_string(party_config->unique_pid));

        return member_idx;
    }
}
