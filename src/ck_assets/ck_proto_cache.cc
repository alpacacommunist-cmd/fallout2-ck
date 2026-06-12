#include "ck_proto_cache.h"
#include "ck_proto_schema.h"
#include "sqlite3.h"
#include <iostream>
#include <algorithm>
#include <cstdio>

#include "proto.h"

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

	sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

	const char* insertSql = "INSERT OR REPLACE INTO protos (pid, fid, type, name, filename, description) VALUES (?, ?, ?, ?, ?, ?);";
	sqlite3_stmt* stmt = nullptr;

	if (sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "[CK SQL Error] Failed to prepare insert statement" << std::endl;
		sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
		return false;
	}

	for (int type = 0; type < fallout::OBJ_TYPE_COUNT; type++) {
		int maxId = fallout::proto_max_id(type);
		if (maxId <= 1) continue;

		for (int id = 0; id < maxId; id++) {
			int pid = (type << 24) | id;
			fallout::Proto* proto = nullptr;

			if (fallout::protoGetProto(pid, &proto) != 0 || proto == nullptr) {
				continue;
			}

			int fid = proto->fid;

			char textBuf[32];
			snprintf(textBuf, sizeof(textBuf), "%08d.pro", id);
			std::string filenameStr(textBuf);

			std::string nameStr = "Proto_" + std::to_string(type) + "_" + std::to_string(id);
			std::string descStr = "Description for proto " + std::to_string(pid);

			sqlite3_bind_int(stmt, 1, pid);
			sqlite3_bind_int(stmt, 2, fid);
			sqlite3_bind_int(stmt, 3, type);
			sqlite3_bind_text(stmt, 4, nameStr.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 5, filenameStr.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 6, descStr.c_str(), -1, SQLITE_TRANSIENT);

			sqlite3_step(stmt);
			sqlite3_reset(stmt);
		}
	}

	sqlite3_finalize(stmt);
	sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

	std::cout << "[CK Proto Cache] Database cache successfully built from engine!" << std::endl;
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

