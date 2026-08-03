#ifndef CK_KNOWLEDGE_H
#define CK_KNOWLEDGE_H

#include "ck_api.h"

#include <string>
#include <vector>

struct CkKnowledgeFFI {
    int id;
    const char* name;
    const char* description;
    int frm_id;
};

struct CkKnowledgeInstance {
    int id;
    std::string name;
    std::string description;
    int frm_id;

    char* ce_name        = nullptr;
    char* ce_description = nullptr;
};

namespace ck::knowledge {
    bool has_any();
    void clear_cache();
    void init_ui();

    const std::vector<CkKnowledgeInstance>& get_player_knowledge();
    void push_cache(const CkKnowledgeFFI& data);
}

CK_API void ck_knowledge_clear_cache();
CK_API void ck_knowledge_push_cache(const CkKnowledgeFFI* data);

#endif
