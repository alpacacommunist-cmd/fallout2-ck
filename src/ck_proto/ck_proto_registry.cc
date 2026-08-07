#include "ck_proto_registry.h"
#include "ck_ids.h"
#include "ck_messages/ck_messages.h"
#include "ck_lua_proxy/ck_lua_proxy_state.h"

#include <vector>
#include <unordered_map>

#include "obj_types.h"

#include <cstring>
#include <memory>
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
        0, 0, 0, 0, 0,
    };
}

extern "C" const char* ck_get_current_mod_id();

namespace ck::proxy::detail {
    extern int receive_proto_list;
}

namespace ck::proto {
    struct MallocDeleter { void operator()(void* ptr) const { std::free(ptr); } };
    using UniqueProtoPtr = std::unique_ptr<fallout::Proto, MallocDeleter>;

    struct CustomProtoNode { int pid; UniqueProtoPtr memory; };

    namespace {
        std::vector<CustomProto> registry_protos; // attribute patches
        std::vector<CustomProtoNode> g_custom_protos; // in-mem protos

        std::unordered_map<int, int> id_translation_table; // id -> pid table; id keeps custom pid
    }

    static void prepare_object_for_save(fallout::Object* object) {
        const CustomProto* proto = find_by_pid(object->pid);
        if (proto) {
            object->id  = object->pid;
            object->pid = proto->source_pid;
        }
    }

    static void restore_object_after_save(fallout::Object* object) {
        auto it = id_translation_table.find(object->id);
        if (it != id_translation_table.end()) {
            object->pid = it->second;
        }
        object->id = 0;
    }

    void sync_custom_items_on_map(SyncMode mode) {
        if (mode == SyncMode::Prepare) {
            std::vector<CustomProtoLuaView> state_vector;
            for (auto& custom_proto : registry_protos)
                state_vector.push_back({custom_proto.pid, custom_proto.lua_tag.c_str()});

            proxy::execute_proxy_call<bool>(proxy::detail::receive_proto_list, state_vector.data(), (int)state_vector.size());
        } else {
            std::vector<proxy::CustomProtoState> state_protos = ck::proxy::get_proto_list();

            for (const auto& proto_state : state_protos) {
                auto it = std::find_if(registry_protos.begin(), registry_protos.end(),
                        [&proto_state](const auto& p) { return p.lua_tag == proto_state.tag; });

                if (it != registry_protos.end()) {
                    id_translation_table[proto_state.id] = it->pid;
                }
            }

            for (auto& proto_state : state_protos) {
                logger.info("proto_tag: {}, proto_id: {}", proto_state.tag, proto_state.id);
            }
        }

        for (int elevation = 0; elevation < 3; elevation++) {
            fallout::Object* object = fallout::objectFindFirstAtElevation(elevation);

            while (object != nullptr) {
                if (mode == SyncMode::Prepare) {
                    if (ck::ids::is_ck_item_pid(object->pid)) {
                        prepare_object_for_save(object);
                    }
                } else { // SyncMode::Restore
                    if (ck::ids::is_ck_item_pid(object->id)) {
                        restore_object_after_save(object);
                    }
                }

                fallout::Inventory* inventory = &(object->data.inventory);
                if (inventory && inventory->items) {
                    for (int i = 0; i < inventory->length; i++) {
                        fallout::Object* item = inventory->items[i].item;
                        if (!item) continue;

                        if (mode == SyncMode::Prepare) {
                            if (ck::ids::is_ck_item_pid(item->pid)) {
                                prepare_object_for_save(item);
                            }
                        } else { // SyncMode::Restore
                            if (ck::ids::is_ck_item_pid(item->id)) {
                                restore_object_after_save(item);
                            }
                        }
                    }
                }

                object = fallout::objectFindNextAtElevation();
            }
        }
    }

    void memory_clear() {
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

            UniqueProtoPtr custom_proto_ptr(custom_proto);
            g_custom_protos.push_back({ current_pid, std::move(custom_proto_ptr) });

            proto.pid = current_pid;

            int msg_name_id = current_pid * 100;
            int msg_desc_id = (current_pid * 100) + 1;

            if (!proto.name.empty()) {
                ck::messages_add_string("pro_item.msg", msg_name_id, proto.name.c_str());
            }

            if (!proto.description.empty()) {
                ck::messages_add_string("pro_item.msg", msg_desc_id, proto.description.c_str());
            }

            if (ck::ids::is_ck_frm(proto.inv_fid)) {
                logger.error("FID: {}", proto.inv_fid);
                item_proto->inventoryFid = proto.inv_fid;
            }

            current_pid++;
        }
    }

    int get_custom_proto(int pid, fallout::Proto** protoPtr) {
        for (const auto& node : g_custom_protos) {
            if (node.pid == pid) {
                *protoPtr = node.memory.get();
                return 0;
            }
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

        proto.inv_fid     = ffi_data.inv_fid;

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
