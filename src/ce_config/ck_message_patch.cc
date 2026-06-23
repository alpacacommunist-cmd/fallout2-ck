#include "ck_message_patch.h"
#include <vector>
#include <iostream>
#include <cstring>
#include <string_view>

#include "ck_log.h"
static const Logger log("CK Message Patch");

static std::vector<CkMessagePatch> gMessagePatches;

void ck_message_patch_add(std::string_view file_path, int num, std::string_view text) {
	std::string normalized_path(file_path);
	for (char& c : normalized_path) if (c == '\\') c = '/';

    gMessagePatches.push_back({ normalized_path, num, std::string(text) });
	log.info("Registered: {{{}}} = {} ({})", num, text, normalized_path);
}

void ck_message_patch_apply(fallout::MessageList* list, const char* file_path) {
    if (list == nullptr || file_path == nullptr) return;

    // normalize path
	std::string check_path = file_path;
    for (char& c : check_path) if (c == '\\') c = '/';

    int applied = 0;
    for (const auto& patch : gMessagePatches) {
        // check the filepath end — "worldmap.msg" matches every path
        if (check_path.find(patch.file_path) != std::string::npos ||
            patch.file_path.find(check_path) != std::string::npos) {

            // static buffers — MessageListItem char*
            // using strdup to keep alive
            fallout::MessageListItem item;
            item.num   = patch.num;
            item.flags = 0;
            item.audio = const_cast<char*>("");
			item.text  = const_cast<char*>(patch.text.c_str());

            fallout::_message_addExternal(list, &item);
			log.debug("Added message num={} text={}", item.num, item.text);
            applied++;
        }
    }

    if (applied > 0) {
		log.info("Applied {} messages to: {}", applied, file_path);
    }
}

void ck_message_patch_clear() {
    gMessagePatches.clear();
}
