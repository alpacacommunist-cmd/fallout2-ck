// include/ck_message_patch.h
#ifndef CK_MESSAGE_PATCH_H
#define CK_MESSAGE_PATCH_H

#include <string>
#include "message.h"

struct CkMessagePatch {
    std::string filePath;  // "worldmap.msg"
    int num;               // {1549}
    std::string text;      // "Test Cave"
};

void ck_message_patch_add(const std::string& filePath, int num, const std::string& text);
void ck_message_patch_apply(fallout::MessageList* list, const char* filePath);
void ck_message_patch_clear();

#endif
