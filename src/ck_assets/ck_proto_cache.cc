#include "ck_proto_cache.h"
#include "ck_proto_schema.h"
#include "sqlite3.h"
#include "proto.h"
#include "message.h"

#include <algorithm>
#include <cstdio>
#include <format>

#include "ck_log.h"
static const Logger log("CK Scripting");

CkProtoCache gProtoCache;

CkProtoCache::CkProtoCache() : db(nullptr) {}

CkProtoCache::~CkProtoCache() {
    if (db) {
        sqlite3_close(db);
    }
}

bool CkProtoCache::initialize(const std::string& cachePath) {
    bool fileExists = false;

    if (sqlite3_open_v2(cachePath.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr) == SQLITE_OK) {
        fileExists = true;
        sqlite3_stmt* stmt = nullptr;
        int currentFileVoid = 0;

        if (sqlite3_prepare_v2(db, "PRAGMA user_version;", -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                currentFileVoid = sqlite3_column_int(stmt, 0);
            }
        }
        sqlite3_finalize(stmt);

        if (currentFileVoid == CK_PROTO_DB_VERSION) {
            log.info("DB version matches ({}). Cache loaded successfully.", CK_PROTO_DB_VERSION);
            return true;
        }

        log.warn("DB version mismatch (File: {}, Code: {}). Rebuilding...", currentFileVoid, CK_PROTO_DB_VERSION);

        sqlite3_close(db);
        db = nullptr;
        std::remove(cachePath.c_str());
    } else {
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
    }

    if (sqlite3_open_v2(cachePath.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
        log.error("Failed to create cache file at: {}", cachePath);
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
        return false;
    }

	if (!createTables()) {
		return false;
	}

    return buildFromEngine();
}

bool CkProtoCache::createTables() {
    char* errMsg = nullptr;

    if (sqlite3_exec(db, CK_PROTO_SCHEMA_SQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        log.error("Failed to create tables: {}", errMsg ? errMsg : "Unknown SQL error");
        if (errMsg) {
            sqlite3_free(errMsg);
        }
        return false;
    }

    const char* const indexNameSql = "CREATE INDEX IF NOT EXISTS idx_protos_fid ON protos(fid);";
    const char* const indexTypeSql = "CREATE INDEX IF NOT EXISTS idx_protos_pid ON protos(pid);";

    sqlite3_exec(db, indexNameSql, nullptr, nullptr, nullptr);
    sqlite3_exec(db, indexTypeSql, nullptr, nullptr, nullptr);

    std::string versionSql = std::format("PRAGMA user_version = {};", CK_PROTO_DB_VERSION);
    sqlite3_exec(db, versionSql.c_str(), nullptr, nullptr, nullptr);

    return true;
}

bool CkProtoCache::buildFromEngine() {
    sqlite3_exec(db, "PRAGMA synchronous = OFF;", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "PRAGMA journal_mode = MEMORY;", nullptr, nullptr, nullptr);

    fallout::MessageList msgLists[fallout::OBJ_TYPE_COUNT];
    bool msgListLoaded[fallout::OBJ_TYPE_COUNT] = { false };

    for (int i = 0; i < fallout::OBJ_TYPE_COUNT; i++) {
        fallout::messageListInit(&msgLists[i]);
    }

    auto loadMsg = [&](int type, const char* filename) {
        std::string fullMsgPath = "game\\" + std::string(filename);
        if (fallout::messageListLoad(&msgLists[type], const_cast<char*>(fullMsgPath.c_str()))) {
            msgListLoaded[type] = true;
            log.info("Successfully loaded text base: {} for type {}", filename, type);
        } else {
            log.error("Failed to load msg file: {}", fullMsgPath);
        }
    };

    loadMsg(fallout::OBJ_TYPE_ITEM,      "pro_item.msg");
    loadMsg(fallout::OBJ_TYPE_CRITTER,   "pro_crit.msg");
    loadMsg(fallout::OBJ_TYPE_SCENERY,   "pro_scen.msg");
    loadMsg(fallout::OBJ_TYPE_WALL,      "pro_wall.msg");
    loadMsg(fallout::OBJ_TYPE_TILE,      "pro_tile.msg");
    loadMsg(fallout::OBJ_TYPE_MISC,      "pro_misc.msg");

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    const char* insertSql = "INSERT OR REPLACE INTO protos (pid, fid, type, sid, name, filename, description) VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr) != SQLITE_OK) {
        log.error("Failed to prepare insert statement");
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        for (int i = 0; i < fallout::OBJ_TYPE_COUNT; i++) fallout::messageListFree(&msgLists[i]);
        return false;
    }

    // Собираем кэш по всем типам объектов
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
            int sid = proto->sid;

            std::string filenameStr = std::format("{:08d}.pro", id);

            std::string nameStr = "";
            std::string descStr = "";

            if (msgListLoaded[type]) {
                fallout::MessageListItem msgItem;

                if (type == fallout::OBJ_TYPE_ITEM || type == fallout::OBJ_TYPE_CRITTER || type == fallout::OBJ_TYPE_SCENERY) {
                    int msgIndex = id - 1;

                    int nameMsgId = 100 + (msgIndex * 100);
                    msgItem.num = nameMsgId;
                    if (fallout::messageListGetItem(&msgLists[type], &msgItem)) nameStr = msgItem.text;

                    int descMsgId = nameMsgId + 1;
                    msgItem.num = descMsgId;
                    if (fallout::messageListGetItem(&msgLists[type], &msgItem)) descStr = msgItem.text;
                } else {
                    msgItem.num = id;
                    if (fallout::messageListGetItem(&msgLists[type], &msgItem)) nameStr = msgItem.text;
                    descStr = "";
                }
            }

            if (nameStr.empty()) {
                nameStr = std::format("Proto_{}_{}", type, id);
            }

            sqlite3_bind_int(stmt, 1, pid);
            sqlite3_bind_int(stmt, 2, fid);
            sqlite3_bind_int(stmt, 3, type);
            sqlite3_bind_int(stmt, 4, sid);

            sqlite3_bind_text(stmt, 5, nameStr.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 6, filenameStr.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 7, descStr.c_str(), -1, SQLITE_STATIC);

            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    for (int i = 0; i < fallout::OBJ_TYPE_COUNT; i++) {
        fallout::messageListFree(&msgLists[i]);
    }

    log.info("Database cache successfully built");
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
