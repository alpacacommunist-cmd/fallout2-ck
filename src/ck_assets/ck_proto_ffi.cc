#include "ck_proto_ffi.h"
#include "ck_proto_cache.h"
#include "sqlite3.h"
#include <string>

// single result buffers
static std::string g_hold_name;
static std::string g_hold_filename;
static std::string g_hold_desc;

static const char* sqlite3_column_text_safe(sqlite3_stmt* stmt, int col) {
    const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, col));
    return text ? text : "";
}

static void fill_proto_info_base(sqlite3_stmt* stmt, CkProtoInfoFFI* out_info) {
    out_info->pid  = sqlite3_column_int(stmt, 0);
    out_info->fid  = sqlite3_column_int(stmt, 1);
    out_info->type = sqlite3_column_int(stmt, 2);
    out_info->sid  = sqlite3_column_int(stmt, 3);
}

static void fill_proto_info_with_holding(sqlite3_stmt* stmt, CkProtoInfoFFI* out_info) {
    fill_proto_info_base(stmt, out_info);

    g_hold_name     = sqlite3_column_text_safe(stmt, 4);
    g_hold_filename = sqlite3_column_text_safe(stmt, 5);
    g_hold_desc     = sqlite3_column_text_safe(stmt, 6);

    out_info->name        = g_hold_name.c_str();
    out_info->filename    = g_hold_filename.c_str();
    out_info->description = g_hold_desc.c_str();
}

bool ck_proto_get_by_pid(int pid, CkProtoInfoFFI* out_info) {
    sqlite3* db = gProtoCache.get_db_handle();
    if (!db || !out_info) return false;

    const char* sql = "SELECT pid, fid, type, sid, name, filename, description FROM protos WHERE pid = ?;";
    sqlite3_stmt* stmt = nullptr;
    bool found = false;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, pid);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            fill_proto_info_with_holding(stmt, out_info);
            found = true;
        }
    }
    sqlite3_finalize(stmt);
    return found;
}

bool ck_proto_get_by_name(const char* name, int type, CkProtoInfoFFI* out_info) {
    sqlite3* db = gProtoCache.get_db_handle();
    if (!db || !name || !out_info) return false;

    std::string sql = "SELECT pid, fid, type, sid, name, filename, description FROM protos WHERE name = ?";
    sql += (type != -1) ? " AND type = ?;" : ";";

    sqlite3_stmt* stmt = nullptr;
    bool found = false;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
        if (type != -1) {
            sqlite3_bind_int(stmt, 2, type);
        }

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            fill_proto_info_with_holding(stmt, out_info);
            found = true;
        }
    }
    sqlite3_finalize(stmt);
    return found;
}

int ck_proto_get_by_type(int type, CkProtoInfoFFI* out_array, int max_count) {
    sqlite3* db = gProtoCache.get_db_handle();
    if (!db || !out_array || max_count <= 0) return 0;

    const char* sql = "SELECT pid, fid, type, sid, name, filename, description FROM protos WHERE type = ?;";
    sqlite3_stmt* stmt = nullptr;
    int count = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, type);

        while (sqlite3_step(stmt) == SQLITE_ROW && count < max_count) {
            CkProtoInfoFFI* out_info = &out_array[count];
            fill_proto_info_base(stmt, out_info);

            out_info->name        = sqlite3_column_text_safe(stmt, 4);
            out_info->filename    = sqlite3_column_text_safe(stmt, 5);
            out_info->description = sqlite3_column_text_safe(stmt, 6);
            count++;
        }
    }
    sqlite3_finalize(stmt);
    return count;
}
