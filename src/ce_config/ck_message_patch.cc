#include "ck_message_patch.h"
#include <vector>
#include <iostream>
#include <cstring>
#include "debug.h"

static std::vector<CkMessagePatch> gMessagePatches;

void ck_message_patch_add(const std::string& filePath, int num, const std::string& text) {
    gMessagePatches.push_back({ filePath, num, text });
    std::cout << "[CK Message Patch] Registered: {" << num << "} = "
              << text << " (" << filePath << ")" << std::endl;
}

void ck_message_patch_apply(fallout::MessageList* list, const char* filePath) {
    if (list == nullptr || filePath == nullptr) return;

    // normalize path
    std::string checkPath = filePath;
    for (char& c : checkPath) if (c == '\\') c = '/';

    int applied = 0;
    for (const auto& patch : gMessagePatches) {
        std::string patchPath = patch.filePath;
        for (char& c : patchPath) if (c == '\\') c = '/';

        // check the filepath end — "worldmap.msg" matches every path
        if (checkPath.find(patchPath) != std::string::npos ||
            patchPath.find(checkPath) != std::string::npos) {

            // static buffers — MessageListItem char*
            // using strdup to keep alive
            fallout::MessageListItem item;
            item.num   = patch.num;
            item.flags = 0;
            item.audio = const_cast<char*>("");
            item.text  = strdup(patch.text.c_str());

            fallout::_message_addExternal(list, &item);
			fallout::debugPrint("[CK DEBUG] Added message num=%d text=%s to list\n",
					item.num, item.text);
            applied++;
        }
    }

    if (applied > 0) {
        std::cout << "[CK Message Patch] Applied " << applied
                  << " messages to: " << filePath << std::endl;
    }
}

void ck_message_patch_clear() {
    gMessagePatches.clear();
}
