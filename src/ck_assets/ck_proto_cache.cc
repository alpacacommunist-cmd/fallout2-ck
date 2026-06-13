#include "ck_proto_cache.h"
#include "ck_proto_schema.h"
#include "sqlite3.h"
#include <iostream>
#include <algorithm>
#include <cstdio>

#include "proto.h"
#include "message.h"

CkProtoCache gProtoCache;

CkProtoCache::CkProtoCache() : db(nullptr) {}

CkProtoCache::~CkProtoCache() {
	if (db) {
		sqlite3_close(db);
	}
}

bool CkProtoCache::initialize(const std::string& cachePath) {
	if (sqlite3_open_v2(cachePath.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr) == SQLITE_OK) {

		sqlite3_stmt* stmt = nullptr;
		int currentFileVoid = 0;

		if (sqlite3_prepare_v2(db, "PRAGMA user_version;", -1, &stmt, nullptr) == SQLITE_OK) {
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				currentFileVoid = sqlite3_column_int(stmt, 0);
			}
		}
		sqlite3_finalize(stmt);

		if (currentFileVoid == CK_PROTO_DB_VERSION) {
			std::cout << "[CK Proto Cache] DB version matches (" << CK_PROTO_DB_VERSION
				<< "). Cache loaded successfully." << std::endl;
			return true;
		}

		std::cout << "[CK Proto Cache] DB version mismatch (File: " << currentFileVoid
			<< ", Code: " << CK_PROTO_DB_VERSION << "). Rebuilding..." << std::endl;

		sqlite3_close(db);
		db = nullptr;

		std::remove(cachePath.c_str());
	}

	return buildFromEngine(cachePath);
}

bool CkProtoCache::createTables() {
	char* errMsg = nullptr;

	if (sqlite3_exec(db, CK_PROTO_SCHEMA_SQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
		std::cerr << "[CK SQL Error] Failed to create tables: " << errMsg << std::endl;
		sqlite3_free(errMsg);
		return false;
	}

	std::string versionSql = "PRAGMA user_version = " + std::to_string(CK_PROTO_DB_VERSION) + ";";
	sqlite3_exec(db, versionSql.c_str(), nullptr, nullptr, nullptr);

	return true;
}

bool CkProtoCache::buildFromEngine(const std::string& cachePath) {
    if (sqlite3_open_v2(cachePath.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
        std::cerr << "[CK SQL Error] Cannot create database file: " << cachePath << std::endl;
        return false;
    }

    if (!createTables()) return false;

    fallout::MessageList msgLists[fallout::OBJ_TYPE_COUNT];
    bool msgListLoaded[fallout::OBJ_TYPE_COUNT] = { false };

    for (int i = 0; i < fallout::OBJ_TYPE_COUNT; i++) {
        fallout::messageListInit(&msgLists[i]);
    }

    auto loadMsg = [&](int type, const char* filename) {
        std::string fullMsgPath = "game\\" + std::string(filename);
        if (fallout::messageListLoad(&msgLists[type], const_cast<char*>(fullMsgPath.c_str()))) {
            msgListLoaded[type] = true;
            std::cout << "[CK Proto Cache] Successfully loaded text base: " << filename << " for Type: " << type << std::endl;
        } else {
            std::cerr << "[CK MSG WARNING] Failed to load msg file: " << fullMsgPath << std::endl;
        }
    };

// --- File: fallout2-ce/src/obj_types.h ---
// 17 | enum ObjectType {
// 18 |     OBJ_TYPE_ITEM,
// 19 |     OBJ_TYPE_CRITTER,
// 20 |     OBJ_TYPE_SCENERY,
// 21 |     OBJ_TYPE_WALL,
// 22 |     OBJ_TYPE_TILE,
// 23 |     OBJ_TYPE_MISC,
// 24 |     OBJ_TYPE_INTERFACE,
// 25 |     OBJ_TYPE_INVENTORY,
// 26 |     OBJ_TYPE_HEAD,
// 27 |     OBJ_TYPE_BACKGROUND,
// 28 |     OBJ_TYPE_SKILLDEX,
// 29 |     OBJ_TYPE_COUNT,
// 30 | };
//
// ls data/text/english/game/*
// data/text/english/game/CMBATAI2.BAK  data/text/english/game/LSGAME.MSG    data/text/english/game/PROTO.MSG
// data/text/english/game/CMBATAI2.msg  data/text/english/game/MAP.MSG       data/text/english/game/pro_wall.msg
// data/text/english/game/COMBATAI.BAK  data/text/english/game/MISC.MSG      data/text/english/game/quests.msg
// data/text/english/game/COMBATAI.MSG  data/text/english/game/OPTIONS.MSG   data/text/english/game/SCRIPT.MSG
// data/text/english/game/COMBAT.MSG    data/text/english/game/PERK.MSG      data/text/english/game/scrname.msg
// data/text/english/game/custom.msg    data/text/english/game/PIPBOY.MSG    data/text/english/game/SKILLDEX.MSG
// data/text/english/game/DBOX.MSG      data/text/english/game/pro_crit.msg  data/text/english/game/SKILL.MSG
// data/text/english/game/EDITOR.MSG    data/text/english/game/pro_item.msg  data/text/english/game/STAT.MSG
// data/text/english/game/INTRFACE.MSG  data/text/english/game/pro_misc.msg  data/text/english/game/TRAIT.MSG
// data/text/english/game/INVENTRY.MSG  data/text/english/game/pro_scen.msg  data/text/english/game/WORLDMAP.MSG
// data/text/english/game/ITEM.MSG      data/text/english/game/pro_tile.msg  data/text/english/game/WORLDMP.MSG


    loadMsg(fallout::OBJ_TYPE_ITEM,      "pro_item.msg"); // 0
    loadMsg(fallout::OBJ_TYPE_CRITTER,   "pro_crit.msg"); // 1
    loadMsg(fallout::OBJ_TYPE_SCENERY,   "pro_scen.msg"); // 2
    loadMsg(fallout::OBJ_TYPE_WALL,      "pro_wall.msg"); // 3
    loadMsg(fallout::OBJ_TYPE_TILE,      "pro_tile.msg"); // 4
    loadMsg(fallout::OBJ_TYPE_MISC,      "pro_misc.msg"); // 5

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    // const char* insertSql = "INSERT OR REPLACE INTO protos (pid, fid, type, name, filename, description) VALUES (?, ?, ?, ?, ?, ?);";
	const char* insertSql = "INSERT OR REPLACE INTO protos (pid, fid, type, sid, name, filename, description) VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[CK SQL Error] Failed to prepare insert statement" << std::endl;
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        for (int i = 0; i < fallout::OBJ_TYPE_COUNT; i++) fallout::messageListFree(&msgLists[i]);
        return false;
    }

    for (int type = 0; type < fallout::OBJ_TYPE_COUNT; type++) {
        int maxId = fallout::proto_max_id(type);
        if (maxId <= 1) continue;

        for (int id = 1; id < maxId; id++) {
            int pid = (type << 24) | id;
            fallout::Proto* proto = nullptr;

            if (fallout::protoGetProto(pid, &proto) != 0 || proto == nullptr) {
                continue;
            }

            int fid = proto->fid;

            char textBuf[32];
            snprintf(textBuf, sizeof(textBuf), "%08d.pro", id);
            std::string filenameStr(textBuf);

            std::string nameStr = "";
            std::string descStr = "";

            if (msgListLoaded[type]) {
                fallout::MessageListItem msgItem;

				int msgIndex = id - 1;
                // name = 100 + (id * 100), desc = 101 + (id * 2)
                int nameMsgId = 100 + (msgIndex * 100);
                msgItem.num = nameMsgId;
                if (fallout::messageListGetItem(&msgLists[type], &msgItem)) {
                    nameStr = msgItem.text;
                }

                int descMsgId = 100 + (msgIndex * 100) + 1;
                msgItem.num = descMsgId;
                if (fallout::messageListGetItem(&msgLists[type], &msgItem)) {
                    descStr = msgItem.text;
                }
            }

			// no text in MSG
            if (nameStr.empty()) {
                nameStr = "Proto_" + std::to_string(type) + "_" + std::to_string(id);
            }

			int sid = proto->sid;

            sqlite3_bind_int(stmt, 1, pid);
            sqlite3_bind_int(stmt, 2, fid);
            sqlite3_bind_int(stmt, 3, type);
            sqlite3_bind_int(stmt, 4, sid);
            sqlite3_bind_text(stmt, 5, nameStr.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 6, filenameStr.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 7, descStr.c_str(), -1, SQLITE_TRANSIENT);

            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    for (int i = 0; i < fallout::OBJ_TYPE_COUNT; i++) {
        fallout::messageListFree(&msgLists[i]);
    }

    std::cout << "[CK Proto Cache] Database cache successfully built" << std::endl;
    return true;
}

CkProtoInfo CkProtoCache::getByPid(int pid) const {
	CkProtoInfo info;
	if (!db) return info;

	const char* sql = "SELECT pid, fid, type, name, filename, description FROM protos WHERE pid = ?;";
	sqlite3_stmt* stmt = nullptr;

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_int(stmt, 1, pid);

		if (sqlite3_step(stmt) == SQLITE_ROW) {
			info.pid = sqlite3_column_int(stmt, 0);
			info.fid = sqlite3_column_int(stmt, 1);
			info.type = sqlite3_column_int(stmt, 2);
			info.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
			info.filename = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
			info.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
		}
	}
	sqlite3_finalize(stmt);
	return info;
}

CkProtoInfo CkProtoCache::getByName(const std::string& name, int type) const {
	CkProtoInfo info;
	if (!db) return info;

	// if type not specified (-1), search by name, otherwise filter by type
	std::string sql = "SELECT pid, fid, type, name, filename, description FROM protos WHERE name = ?";
	if (type != -1) {
		sql += " AND type = ?;";
	} else {
		sql += ";";
	}

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
		if (type != -1) {
			sqlite3_bind_int(stmt, 2, type);
		}

		if (sqlite3_step(stmt) == SQLITE_ROW) {
			info.pid = sqlite3_column_int(stmt, 0);
			info.fid = sqlite3_column_int(stmt, 1);
			info.type = sqlite3_column_int(stmt, 2);
			info.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
			info.filename = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
			info.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
		}
	}
	sqlite3_finalize(stmt);
	return info;
}

std::vector<CkProtoInfo> CkProtoCache::getByType(int type) const {
	std::vector<CkProtoInfo> results;
	if (!db) return results;

	const char* sql = "SELECT pid, fid, type, name, filename, description FROM protos WHERE type = ?;";
	sqlite3_stmt* stmt = nullptr;

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_int(stmt, 1, type);

		while (sqlite3_step(stmt) == SQLITE_ROW) {
			CkProtoInfo info;
			info.pid = sqlite3_column_int(stmt, 0);
			info.fid = sqlite3_column_int(stmt, 1);
			info.type = sqlite3_column_int(stmt, 2);
			info.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
			info.filename = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
			info.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
			results.push_back(info);
		}
	}
	sqlite3_finalize(stmt);
	return results;
}

