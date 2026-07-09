#ifndef CK_MESSAGES_H
#define CK_MESSAGES_H

#include <string>

namespace fallout {
    struct MessageList;
}

namespace ck {
    void messages_add_string(const std::string& msg_file, int msg_id, const std::string& text);
    void messages_clear();

    void messages_on_list_loaded(fallout::MessageList* list, const char* path);
    void messages_on_list_free(fallout::MessageList* list);
    const char* messages_on_get_message(fallout::MessageList* list, int num, const char* default_text);
}

#endif
