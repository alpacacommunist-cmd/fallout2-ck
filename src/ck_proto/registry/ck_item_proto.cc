#include "utils.h"

#include "ck_ids.h"
#include "ck_lua_proxy/ck_lua_proxy_state.h"
#include "engine/messages.h"
#include "ck_proto_registry.h"

#include <unordered_map>
#include <vector>

#include "fallout2-ce/obj_types.h"

#include "log.h"
static constexpr logging::Context logger{"ck_item_proto.cc"};

namespace fallout {
    Object* objectFindFirstAtElevation(MapElevation elevation);
    Object* objectFindNextAtElevation();
    int objectSetScriptFromProto(Object* object, int* sidPtr);
}

namespace ck::proto::registry::item {
    using namespace ck::utils::fallout;

    namespace {
        // item's mod-defined custom pid is temporarily stored in object's `id`
        // field and is recovered back to pid on game load
        std::unordered_map<int, int> id_translation_table;
    }

    static void prepare_item_for_save(fallout::Object* object) {
        Proto* proto = ck::proto::registry::get(object->pid);
        if (!proto || !ck::ids::is_item(object)) return;

        object->id  = object->pid;
        object->pid = proto->source_pid;
        object->sid = -1;
        object->scriptIndex = proto->source_sid;

        if (proto->source_fid != proto->get_fid()) object->fid = proto->source_fid;
    }

    static void restore_object_after_save(fallout::Object* object) {
        auto it = id_translation_table.find(object->id);
        if (it != id_translation_table.end()) {
            Proto* proto = ck::proto::registry::get(it->second);
            if (!proto || !ck::ids::is_item(object)) return;

            object->pid = proto->pid;
            object->scriptIndex = proto->get_sid();
            object->id = 0;

            fallout::objectSetScriptFromProto(object, &object->sid);
            logger.info.banner("restored sid: {}, index: {}", object->sid, object->scriptIndex);

            if (proto->source_fid != proto->get_fid()) object->fid = proto->get_fid();
        }
    }

    void sync_custom_items_on_map(registry::SyncMode mode) {
        if (mode == registry::SyncMode::Prepare) { // (on game save)
            std::vector<ck::proxy::ItemProtoLuaView> state_vector;

            auto callback = [](const Proto& item_proto, void* user_data) {
                auto* vector = static_cast<std::vector<ck::proxy::ItemProtoLuaView>*>(user_data);
                vector->push_back({item_proto.pid, item_proto.lua_tag.c_str()});
            };

            registry::for_each_item(callback, &state_vector);
            proxy::receive_proto_list(state_vector.data(), static_cast<int>(state_vector.size()));
        } else { // SyncMode::Restore (on game load)
            std::vector<proxy::CustomProtoState> state_protos = ck::proxy::get_proto_list();

            // (quick ref map)
            std::unordered_map<std::string, int> tag_to_current_pid;
            auto callback = [](const types::Proto& item_proto, void* user_data) {
                auto* map = static_cast<std::unordered_map<std::string, int>*>(user_data);
                (*map)[item_proto.lua_tag] = item_proto.pid;
            };
            registry::for_each_item(callback, &tag_to_current_pid);

            for (const auto& proto_state : state_protos) {
                auto it = tag_to_current_pid.find(proto_state.tag);
                if (it != tag_to_current_pid.end()) {
                    id_translation_table[proto_state.id] = it->second;
                    logger.info("{} -> {}", proto_state.id, it->second);
                }

                logger.info("proto_tag: {}, proto_id: {}", proto_state.tag, proto_state.id);
            }
        }

        for (int elevation = 0; elevation < 3; elevation++) {
            fallout::Object* object = fallout::objectFindFirstAtElevation(fallout::MapElevation(elevation));

            while (object != nullptr) {
                if (mode == registry::SyncMode::Prepare) {
                    if (ck::ids::is_ck_pid(object->pid)) {
                        prepare_item_for_save(object);
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

                        if (mode == registry::SyncMode::Prepare) {
                            if (ck::ids::is_ck_pid(item->pid)) {
                                prepare_item_for_save(item);
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

    int allocate(int source_pid, const char *lua_tag, const ItemProtoFFI &ffi_data) {
        Proto *proto = registry::build_generic_prototype(source_pid, lua_tag);
        if (!proto) return -1;

        auto item = proto->as_item();

        if (ffi_data.weight != -1) item.set_weight(ffi_data.weight);
        if (ffi_data.price != -1) item.set_price(ffi_data.price);

        if (ffi_data.inv_fid && ck::ids::is_ck_frm(ffi_data.inv_fid)) {
            item.set_inventory_fid(ffi_data.inv_fid);
        }

        if (ffi_data.ground_fid && ck::ids::is_ck_frm(ffi_data.ground_fid)) {
            proto->set_fid(ffi_data.ground_fid);
        }

        item.set_can_use(ffi_data.usable);

        int msg_name_id = proto->get_msg_id();
        if (!utils::is_blank(ffi_data.name)) {
            ck::messages_add_string("pro_item.msg", msg_name_id, ffi_data.name);
        }

        if (!utils::is_blank(ffi_data.description)) {
            ck::messages_add_string("pro_item.msg", msg_name_id + 1, ffi_data.description);
        }

        return proto->packed_pid();
    }
}

// FFI
int ck_item_proto_register(int source_pid, const char* lua_tag, const ItemProtoFFI* ffi_data) {
    if (!lua_tag || !ffi_data) return -1;

    return ck::proto::registry::item::allocate(source_pid, lua_tag, *ffi_data);
}

