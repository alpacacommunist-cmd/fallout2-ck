#ifndef CK_MESSAGES_H
#define CK_MESSAGES_H

#include <string_view>

namespace fallout {
    struct MessageList;
}

namespace ck {
    bool messages_has_string(std::string_view msg_file, int msg_id);
    void messages_add_string(std::string_view msg_file, int msg_id, std::string_view text);
    void messages_clear();

    void messages_on_list_loaded(fallout::MessageList* list, const char* path);
    void messages_on_list_free(fallout::MessageList* list);
    const char* messages_on_get_message(fallout::MessageList* list, int num, const char* default_text);
}

#endif
