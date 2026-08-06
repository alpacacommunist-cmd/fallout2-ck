#include "ck_proto_registry.h"
#include <vector>
#include <unordered_map>

#include "proto_types.h"
#include "obj_types.h"

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
        std::vector<CustomProto> registry_protos;
        std::unordered_map<fallout::Object*, int> tracked_items;
    }

    void registry_clear() {
        registry_protos.clear();
    }

    void track_item(fallout::Object* item_ptr, int pid) {
        if (!item_ptr) return;
        tracked_items[item_ptr] = pid;

        logger.debug("tracking item: {}", pid);
    }

    void untrack_item(fallout::Object* item_ptr) {
        tracked_items.erase(item_ptr);
    }

    void clear_tracked_items() {
        tracked_items.clear();
    }

    void restore_source_pids() {
        for (auto& [item_ptr, custom_pid] : tracked_items) {
            if (!item_ptr) continue;

            const CustomProto* proto = find_by_pid(custom_pid);
            if (proto) {
                item_ptr->id = custom_pid;
                item_ptr->pid = proto->source_pid;
            }
        }
    }

    void reapply_custom_pids() {
        for (auto& [item_ptr, custom_pid] : tracked_items) {
            if (!item_ptr) continue;

            item_ptr->pid = custom_pid;
            item_ptr->id = 0;
        }
    }

    void rebuild_custom_prototypes() {
        for (auto& proto : registry_protos) {
            int unique_pid = 0;
            if (fallout::proto_new(&unique_pid, proto.object_type) != 0) continue;
            if (fallout::proto_copy_proto(proto.source_pid, unique_pid) != 0) continue;

            fallout::Proto* generic_proto = nullptr;
            if (fallout::protoGetProto(unique_pid, &generic_proto) == 0 && generic_proto != nullptr) {
                fallout::ItemProto* item_proto = (fallout::ItemProto*)generic_proto;

                item_proto->cost   = proto.price;
                item_proto->weight = proto.weight;
            }

            proto.pid = unique_pid;
        }
    }

    void rebuild_tracked_items() {
        clear_tracked_items();
        rebuild_custom_prototypes();

        for (int elevation = 0; elevation < 3; elevation++) {
            fallout::Object* object = fallout::objectFindFirstAtElevation(elevation);

            while (object != nullptr) {
                if (ck::proto::find_by_pid(object->id)) {
                    logger.debug("Re-tracking map object from storage ID: {}", object->id);

                    track_item(object, object->id);
                    object->pid = object->id;
                    object->id  = 0;
                }

                fallout::Inventory* inventory = &(object->data.inventory);
                if (inventory && inventory->items) {
                    for (int i = 0; i < inventory->length; i++) {
                        fallout::Object* item = inventory->items[i].item;

                        if (item && ck::proto::find_by_pid(item->id)) {
                            logger.debug("Re-tracking inventory item from storage ID: {}", item->id);

                            track_item(item, item->id);
                            item->pid = item->id;
                            item->id  = 0;
                        }
                    }
                }

                object = fallout::objectFindNextAtElevation();
            }
        }

        logger.info("Custom items tracking registry successfully rebuilt via engine iterators.");
    }

    int register_prototype(int source_pid, int object_type, const char* lua_tag, const CustomProtoFFI& ffi_data) {
        std::string tag(lua_tag);

        for (auto& proto : registry_protos) {
            if (proto.lua_tag == tag) {
                return proto.pid;
            }
        }

        CustomProto proto;
        proto.pid         = 0;
        proto.source_pid  = source_pid;
        proto.object_type = object_type;
        proto.weight      = ffi_data.weight;
        proto.price       = ffi_data.price;
        proto.lua_tag     = tag;
        proto.mod_id      = ck_get_current_mod_id();

        registry_protos.push_back(proto);

        rebuild_custom_prototypes();

        for (const auto& p : registry_protos) {
            if (p.lua_tag == tag) return p.pid;
        }

        return -1;
    }

    int get_pid_by_tag(const std::string& lua_tag) {
        for (const auto& proto : registry_protos) {
            if (proto.lua_tag == lua_tag) return proto.pid;
        }
        return -1;
    }

    const CustomProto* find_by_pid(int runtime_pid) {
        for (const auto& proto : registry_protos) {
            if (proto.pid == runtime_pid) return &proto;
        }
        return nullptr;
    }

    const std::vector<CustomProto>& get_all_protos() {
        return registry_protos;
    }
}

int ck_proto_register(int source_pid, int object_type, const char* lua_tag, const ck::proto::CustomProtoFFI* ffi_data) {
    if (!lua_tag || !ffi_data) return -1;
    return ck::proto::register_prototype(source_pid, object_type, lua_tag, *ffi_data);
}

int ck_proto_get_pid_by_tag(const char* lua_tag) {
    if (!lua_tag) return -1;
    return ck::proto::get_pid_by_tag(std::string(lua_tag));
}
