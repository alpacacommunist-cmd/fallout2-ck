#include "ck_proto_registry.h"
#include "ck_ids.h"
#include "ck_messages/ck_messages.h"

#include <vector>
#include <unordered_map>

#include "obj_types.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "ck_log.h"
static const Logger logger("CK Item");

namespace fallout {
    int proto_new(int* pid, int object_type);
    int proto_copy_proto(int source_pid, int destination_pid);
    int protoGetProto(int pid, fallout::Proto** proto);

    const size_t _proto_sizes[11] = {
        sizeof(ItemProto),    // 0x84
        sizeof(CritterProto), // 0x1A0
        sizeof(SceneryProto), // 0x38
        sizeof(WallProto),    // 0x24
        sizeof(TileProto),    // 0x1C
        sizeof(MiscProto),    // 0x1C
        0,
        0,
        0,
        0,
        0,
    };
}

extern "C" const char* ck_get_current_mod_id();

namespace ck::proto {
    struct CustomProtoNode {
        int pid;
        fallout::Proto* memory;
    };

    namespace {
        std::vector<CustomProto> registry_protos;
        std::vector<CustomProtoNode> g_custom_protos; // in-mem protos
        std::unordered_map<fallout::Object*, int> tracked_items;
    }

    void sync_custom_items_on_map(SyncMode mode) {
        for (int elevation = 0; elevation < 3; elevation++) {
            fallout::Object* object = fallout::objectFindFirstAtElevation(elevation);

            while (object != nullptr) {
                if (mode == SyncMode::Prepare) {
                    if (ck::ids::is_ck_item_pid(object->pid)) {
                        const CustomProto* proto = find_by_pid(object->pid);
                        if (proto) {
                            object->id  = object->pid;
                            object->pid = proto->source_pid;
                        }
                    }
                } else {
                    if (ck::ids::is_ck_item_pid(object->id)) {
                        object->pid = object->id;
                        object->id  = 0;
                    }
                }

                fallout::Inventory* inventory = &(object->data.inventory);
                if (inventory && inventory->items) {
                    for (int i = 0; i < inventory->length; i++) {
                        fallout::Object* item = inventory->items[i].item;
                        if (!item) continue;

                        if (mode == SyncMode::Prepare) {
                            if (ck::ids::is_ck_item_pid(item->pid)) {
                                const CustomProto* proto = find_by_pid(item->pid);
                                if (proto) {
                                    item->id = item->pid;
                                    item->pid = proto->source_pid;
                                }
                            }
                        } else { // Restore
                            if (ck::ids::is_ck_item_pid(item->id)) {
                                item->pid = item->id;
                                item->id  = 0;
                            }
                        }
                    }
                }

                object = fallout::objectFindNextAtElevation();
            }
        }
    }

    void memory_clear() {
        for (auto& node : g_custom_protos) {
            std::free(node.memory);
        }
        g_custom_protos.clear();
    }

    void registry_clear() {
        registry_protos.clear();
    }

    void rebuild_custom_prototypes() {
        memory_clear();
        int current_pid = ck::ids::CK_ITEM_PID_START;

        for (auto& proto : registry_protos) {
            if (current_pid >= ck::ids::CK_ITEM_PID_LIMIT) break;

            fallout::Proto* src_proto = nullptr;
            if (fallout::protoGetProto(proto.source_pid, &src_proto) != 0 || src_proto == nullptr) {
                continue;
            }

            size_t proto_size = fallout::_proto_sizes[ck::ids::object_types::ITEM];
            void* allocated_mem = std::malloc(proto_size);

            std::memcpy(allocated_mem, src_proto, proto_size);
            fallout::Proto* custom_proto = static_cast<fallout::Proto*>(allocated_mem);

            custom_proto->pid = current_pid;
            custom_proto->messageId = current_pid * 100;

            fallout::ItemProto* item_proto = reinterpret_cast<fallout::ItemProto*>(custom_proto);
            item_proto->cost   = proto.price;
            item_proto->weight = proto.weight;

            g_custom_protos.push_back({ current_pid, custom_proto });

            proto.pid = current_pid;

            int msg_name_id = current_pid * 100;
            int msg_desc_id = (current_pid * 100) + 1;

            if (!proto.name.empty()) {
                ck::messages_add_string("pro_item.msg", msg_name_id, proto.name.c_str());
            }

            if (!proto.description.empty()) {
                ck::messages_add_string("pro_item.msg", msg_desc_id, proto.description.c_str());
            }

            current_pid++;
        }
    }

    int get_custom_proto(int pid, fallout::Proto** protoPtr) {
        auto it = std::find_if(g_custom_protos.begin(), g_custom_protos.end(),
                [pid](const auto& node) { logger.info("proto request: {}", pid); return node.pid == pid; });

        if (it != g_custom_protos.end()) {
            *protoPtr = it->memory;
            return 0;
        }

        return -1;
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

        proto.name        = std::string(ffi_data.name);
        proto.description = std::string(ffi_data.description);

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
