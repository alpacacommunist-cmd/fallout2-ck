#include "ck_knowledge.h"
#include "ck_lua_proxy/ck_lua_proxy.h"
#include "memory.h"
#include <cstring>

namespace ck::proxy::detail {
    extern int knowledge_sync;
}

namespace ck::knowledge {
    static std::vector<CkKnowledgeInstance> g_knowledge_view_cache;

    bool has_any() {
        return !g_knowledge_view_cache.empty();
    }

    void clear_cache() {
        for (auto& kn : g_knowledge_view_cache) {
            if (kn.ce_name) fallout::internal_free(kn.ce_name);
            if (kn.ce_description) fallout::internal_free(kn.ce_description);
        }

        g_knowledge_view_cache.clear();
    }

    void init_ui() {
        clear_cache();
        proxy::execute_proxy_call<bool>(ck::proxy::detail::knowledge_sync);
    }

    const std::vector<CkKnowledgeInstance>& get_player_knowledge() {
        return g_knowledge_view_cache;
    }

    void push_cache(const CkKnowledgeFFI& data) {
        CkKnowledgeInstance instance;
        instance.id     = data.id;
        instance.frm_id = data.frm_id;

        instance.name = data.name ? data.name : std::string();
        instance.description = data.description ? data.description : std::string();

        if (!instance.name.empty()) {
            instance.ce_name = (char*)fallout::internal_malloc(instance.name.size() + 1);
            if (instance.ce_name) {
                memcpy(instance.ce_name, instance.name.c_str(), instance.name.size() + 1);
            }
        }

        if (!instance.description.empty()) {
            instance.ce_description = (char*)fallout::internal_malloc(instance.description.size() + 1);
            if (instance.ce_description) {
                memcpy(instance.ce_description, instance.description.c_str(), instance.description.size() + 1);
            }
        }

        g_knowledge_view_cache.push_back(instance);
    }
}

void ck_knowledge_clear_cache() {
    ck::knowledge::clear_cache();
}

void ck_knowledge_push_cache(const CkKnowledgeFFI* data) {
    if (data) ck::knowledge::push_cache(*data);
}
