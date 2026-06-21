// src/ce_config/ck_message_patch.h
#ifndef CK_MESSAGE_PATCH_H
#define CK_MESSAGE_PATCH_H

#include <string>
#include "message.h"

struct CkMessagePatch {
    std::string file_path;  // "worldmap.msg"
    int num;               // {1549}
    std::string text;      // "Test Cave"
};

void ck_message_patch_add(std::string_view file_path, int num, std::string_view text);
void ck_message_patch_apply(fallout::MessageList* list, const char* file_path);
void ck_message_patch_clear();

#endif
