#include "ck_proto_registry.h"
#include <unordered_map>

#include "proto_types.h"
#include "item.h"

#include "ck_log.h"
static const Logger logger("CK Item");

namespace fallout {
    int proto_new(int* pid, int object_type);
    int proto_copy_proto(int source_pid, int destination_pid);
    int protoGetProto(int pid, fallout::Proto** proto);
}

extern "C" const char* ck_get_current_mod_id();

namespace ck::proto {
    namespace {
        std::unordered_map<int, CustomProtoInfo> registry_by_pid;
        std::unordered_map<std::string, int> tag_to_pid_mapping;
    }

    void registry_init() {
        registry_clear();
    }

    void registry_clear() {
        registry_by_pid.clear();
        tag_to_pid_mapping.clear();

        logger.debug("Registry cleared");
    }

    int register_prototype(int source_pid, int object_type, const std::string& lua_tag) {
        auto it = tag_to_pid_mapping.find(lua_tag);
        if (it != tag_to_pid_mapping.end()) {
            return it->second;
        }

        int pid = 0;
        if (fallout::proto_new(&pid, object_type) != 0) {
            return -1;
        }

        if (fallout::proto_copy_proto(source_pid, pid) != 0) {
            return -1;
        }

        CustomProtoInfo info;
        info.pid = pid;
        info.source_pid = source_pid;
        info.object_type = object_type;
        info.lua_tag = lua_tag;
        info.mod_id = ck_get_current_mod_id();

        registry_by_pid[pid] = info;
        tag_to_pid_mapping[lua_tag] = pid;

        logger.info("registered new pid: {}", pid);
        return pid;
    }

    const CustomProtoInfo* find_by_pid(int pid) {
        auto it = registry_by_pid.find(pid);
        return (it != registry_by_pid.end()) ? &it->second : nullptr;
    }

    void convert_custom_items_to_base(fallout::Object* critter) {
        if (!critter) return;

        fallout::Inventory* inventory = &(critter->data.inventory);
        if (!inventory || !inventory->items) return;

        for (int i = 0; i < inventory->length; i++) {
            fallout::Object* item = inventory->items[i].item;
            if (!item) continue;

            const CustomProtoInfo* info = find_by_pid(item->pid);
            if (info) {
                item->id  = item->pid;
                item->pid = info->source_pid;

                logger.debug("Saved custom item {} as base PID {}", info->lua_tag, info->source_pid);
            }
        }
    }

    void restore_custom_items_from_base(fallout::Object* critter) {
        if (!critter) return;

        fallout::Inventory* inventory = &(critter->data.inventory);
        if (!inventory || !inventory->items) return;

        for (int i = 0; i < inventory->length; i++) {
            fallout::Object* item = inventory->items[i].item;
            if (!item) continue;

            if (find_by_pid(item->id)) {
                item->pid = item->id;
                item->id  = 0;

                logger.debug("Restored custom item PID {}", item->pid);
            }
        }
    }
}


int ck_proto_register(int base_pid, int object_type, const char* lua_tag) {
    if (!lua_tag) return -1;
    return ck::proto::register_prototype(base_pid, object_type, std::string(lua_tag));
}

int ck_proto_set_item_cost(int pid, int new_cost) {
    fallout::Proto* generic_proto = nullptr;
    if (fallout::protoGetProto(pid, &generic_proto) != 0 || generic_proto == nullptr) {
        return -1;
    }

    fallout::ItemProto* item_proto = (fallout::ItemProto*)generic_proto;
    item_proto->cost = new_cost;
    return 0;
}

int ck_proto_set_item_weight(int pid, int new_weight) {
    fallout::Proto* generic_proto = nullptr;
    if (fallout::protoGetProto(pid, &generic_proto) != 0 || generic_proto == nullptr) {
        return -1;
    }

    fallout::ItemProto* item_proto = (fallout::ItemProto*)generic_proto;
    item_proto->weight = new_weight;
    return 0;
}
