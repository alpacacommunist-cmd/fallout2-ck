#include "ck_utils.h"
#include "ck_ids.h"
#include "ck_proto_registry.h"
#include "ck_lua_proxy/ck_lua_proxy_state.h"
#include "ck_messages/ck_messages.h"

#include <vector>
#include <unordered_map>

#include "ck_log.h"
static const Logger logger("CK Proto Item");

namespace fallout {
    Object* objectFindFirstAtElevation(int elevation);
    Object* objectFindNextAtElevation();
}

namespace ck::proto::item {
    static int fallout_object_type = static_cast<int>(ids::ObjectType::ITEM);

    namespace {
        // item's mod-defined custom pid is temporarily stored in object's `id`
        // field and is recovered back to pid on game load
        std::unordered_map<int, int> id_translation_table;
        // memory
        std::vector<ProtoNode> g_item_protos;
        // attributes
        std::vector<ItemProto> registry_item_protos;
        // mod -> pid
        std::unordered_map<std::string, std::vector<int>> g_mod_allocated_item_pids;
    } 

    int get_proto(int pid, fallout::Proto** protoPtr) {
        for (const auto& node : g_item_protos) {
            if (node.pid == pid) {
                *protoPtr = node.memory.get();
                return 0;
            }
        }

        return -1;
    }

    bool has_pid(int pid) {
        fallout::Proto* proto;
        return get_proto(pid, &proto) == 0;
    }

    const ItemProto* find_by_pid(int pid) {
        for (const auto& proto : registry_item_protos) {
            if (proto.pid == pid) return &proto;
        }
        return nullptr;
    }

    int get_pid_by_tag(const std::string& lua_tag) {
        for (const auto& proto : registry_item_protos) {
            if (proto.lua_tag == lua_tag) return proto.pid;
        }
        return -1;
    }

    static int store_prototype_patch(int source_pid, const char* lua_tag, const ItemProtoFFI& ffi_data) {
        std::string tag(lua_tag);
        int pid = ck::proto::next_proto_pid();

        if (!ck::proto::is_supported_proto_type(fallout_object_type)) {
            logger.error("Cannot register prototype '{}': Unsupported or invalid object type {}!", tag, fallout_object_type);
            return -1;
        }

        for (auto& proto : registry_item_protos) {
            if (proto.lua_tag == tag) return proto.pid;
        }

        if (pid >= ck::ids::CK_PID_LIMIT) {
            logger.error("Cannot register prototype '{}': Custom item PIDs limit reached!", tag);
            return -1;
        }

        ItemProto proto;
        proto.pid         = pid;
        proto.source_pid  = source_pid;

        proto.weight      = ffi_data.weight;
        proto.price       = ffi_data.price;

        proto.name        = std::string(ffi_data.name);
        proto.description = std::string(ffi_data.description);

        proto.inv_fid     = ffi_data.inv_fid;
        proto.ground_fid  = ffi_data.ground_fid;

        proto.usable      = ffi_data.usable;

        proto.lua_tag     = tag;
        proto.mod_id      = std::string(ck::common::current_mod_id());

        registry_item_protos.push_back(proto);

        return pid;
    }

    static void prepare_object_for_save(fallout::Object* object) {
        const ItemProto* proto = find_by_pid(object->pid);
        if (proto) {
            object->id  = object->pid;
            object->pid = proto->source_pid;

            fallout::Proto* source_proto = nullptr;
            if (fallout::protoGetProto(proto->source_pid, &source_proto) == 0 && source_proto) {
                object->sid = source_proto->sid;
            } else {
                object->sid = -1;
            }
        }
    }

    static void restore_object_after_save(fallout::Object* object) {
        auto it = id_translation_table.find(object->id);
        if (it != id_translation_table.end()) {
            int pid = it->second;

            object->pid = pid;

            fallout::Proto* proto = nullptr;
            if (fallout::protoGetProto(pid, &proto) == 0 && proto) {
                object->sid = proto->sid;
            }
        }

        object->id = 0;
    }

    void sync_custom_items_on_map(SyncMode mode) {
        if (mode == SyncMode::Prepare) { // (on game save)
            std::vector<ck::proxy::ItemProtoLuaView> state_vector;
            for (auto& item_proto : registry_item_protos)
                state_vector.push_back({item_proto.pid, item_proto.lua_tag.c_str()});

            proxy::receive_proto_list(state_vector.data(), static_cast<int>(state_vector.size()));
        } else { // SyncMode::Restore (on game load)
            std::vector<proxy::CustomProtoState> state_protos = ck::proxy::get_proto_list();

            for (const auto& proto_state : state_protos) {
                auto it = std::find_if(registry_item_protos.begin(), registry_item_protos.end(),
                        [&proto_state](const auto& p) { return p.lua_tag == proto_state.tag; });

                if (it != registry_item_protos.end()) {
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
                    if (ck::ids::is_ck_pid(object->pid)) {
                        prepare_object_for_save(object);
                    }
                } else { // SyncMode::Restore
                    if (ck::ids::is_ck_pid(object->id)) {
                        restore_object_after_save(object);
                    }
                }

                fallout::Inventory* inventory = &(object->data.inventory);
                if (inventory && inventory->items) {
                    for (int i = 0; i < inventory->length; i++) {
                        fallout::Object* item = inventory->items[i].item;
                        if (!item) continue;

                        if (mode == SyncMode::Prepare) {
                            if (ck::ids::is_ck_pid(item->pid)) {
                                prepare_object_for_save(item);
                            }
                        } else { // SyncMode::Restore
                            if (ck::ids::is_ck_pid(item->id)) {
                                restore_object_after_save(item);
                            }
                        }
                    }
                }

                object = fallout::objectFindNextAtElevation();
            }
        }
    }

    int register_item_prototype(int source_pid, const char* lua_tag, const ItemProtoFFI& ffi_data) {
        int pid = store_prototype_patch(source_pid, lua_tag, ffi_data);
        if (pid == -1) {
            logger.error("Failed to save prototype input data {}", lua_tag);
            return -1;
        }

        fallout::Proto* generic_proto = ck::proto::build_generic_prototype(source_pid, pid, fallout_object_type);

        if (generic_proto == nullptr) {
            logger.error("Failed to allocate memory for ITEM prototype");
            return -1;
        }

        auto* item_proto = reinterpret_cast<fallout::ItemProto*>(generic_proto);

        item_proto->pid = pid;

        item_proto->cost   = ffi_data.price;
        item_proto->weight = ffi_data.weight;

        if (ffi_data.usable) {
            item_proto->extendedFlags |= fallout::ProtoExtendedFlags::PROTO_EXT_FLAG_CAN_USE;
        }

        if (ck::ids::is_ck_frm(ffi_data.inv_fid)) {
            item_proto->inventoryFid = ffi_data.inv_fid;
        }

        if (ck::ids::is_ck_frm(ffi_data.ground_fid)) {
            item_proto->fid = ffi_data.ground_fid;
        }

        int msg_name_id = pid * 100;

        if (!utils::is_blank(ffi_data.name)) {
            ck::messages_add_string("pro_item.msg", msg_name_id, ffi_data.name);
        }

        if (!utils::is_blank(ffi_data.description)) {
            ck::messages_add_string("pro_item.msg", msg_name_id + 1, ffi_data.description);
        }

        g_item_protos.push_back({ pid, UniqueProtoPtr(generic_proto) });
        return pid;
    }

    void clear() {
        // id -> pid
        id_translation_table.clear();
        // memory
        g_item_protos.clear();
        // attributes
        registry_item_protos.clear();
        // mod -> pid
        g_mod_allocated_item_pids.clear();
    }

    void clear_for_mod(const std::string& mod_id) {
        auto it = g_mod_allocated_item_pids.find(mod_id);

        if (it != g_mod_allocated_item_pids.end()) {
            for (int pid : it->second) {
                fallout::Proto* proto = nullptr;
                fallout::protoGetProto(pid, &proto);
                if (proto) {
                    ck::proto::unlink_sid(proto->sid);
                }

                std::erase_if(g_item_protos, [pid](const ProtoNode& node) { return node.pid == pid; });
                std::erase_if(registry_item_protos, [pid](const ItemProto& p) { return p.pid == pid; });
            }
            g_mod_allocated_item_pids.erase(it);
            logger.debug("Freed ITEM prototypes for mod: {}", mod_id);
        }
    }
}

// ffi
int ck_item_proto_register(int source_pid, const char* lua_tag, const ItemProtoFFI* ffi_data) {
    if (!lua_tag || !ffi_data) return -1;

    return ck::proto::item::register_item_prototype(source_pid, lua_tag, *ffi_data);
}

