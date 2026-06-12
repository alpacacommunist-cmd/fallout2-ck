#include "ck_proto_cache.h"
#include "sqlite3.h" // Наш подключенный single-file вендор
#include <iostream>
#include <algorithm>

#include "proto.h"

// Подключаем необходимые заголовочные файлы движка для получения типов и прототипов
// Примечание: Убедись, что эти пути и названия функций совпадают с твоим форком fallout2-ce!
namespace fallout {
    #include "object.h" // Для OBJ_TYPE_COUNT, Proto и т.д.
}

CkProtoCache gProtoCache;

CkProtoCache::CkProtoCache() : db(nullptr) {}

CkProtoCache::~CkProtoCache() {
    if (db) {
        sqlite3_close(db);
    }
}

bool CkProtoCache::initialize(const std::string& cachePath) {
    // Попробуем открыть существующую БД
    // SQLITE_OPEN_READWRITE откроет файл, только если он есть. Если его нет — вернет ошибку.
    if (sqlite3_open_v2(cachePath.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr) == SQLITE_OK) {
        std::cout << "[CK Proto Cache] Successfully opened existing SQLite DB: " << cachePath << std::endl;
        return true;
    }

    // Если не открылось (файла нет), закрываем дескриптор ошибки и строим с нуля
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }

    std::cout << "[CK Proto Cache] DB not found. Building fresh proto cache from engine..." << std::endl;
    return buildFromEngine(cachePath);
}

bool CkProtoCache::createTables() {
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS protos ("
        "pid INTEGER PRIMARY KEY, "
        "fid INTEGER, "
        "type INTEGER, "
        "name TEXT, "
        "filename TEXT, "
        "description TEXT);";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "[CK SQL Error] Failed to create tables: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool CkProtoCache::buildFromEngine(const std::string& cachePath) {
    // Открываем БД в режиме создания файла
	if (sqlite3_open_v2(cachePath.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
        std::cerr << "[CK SQL Error] Cannot create database file: " << cachePath << std::endl;
        return false;
    }

    if (!createTables()) return false;

    // Включаем транзакцию — КРИТИЧЕСКИ важная штука для веб-разработчика.
    // Без транзакции SQLite будет записывать каждый инсерт на жесткий диск отдельно, и цикл займет 20 секунд.
    // С транзакцией всё запишется в буфер оперативки и сбросится на диск ОДНИМ махом за 0.05 секунды!
    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    const char* insertSql = "INSERT OR REPLACE INTO protos (pid, fid, type, name, filename, description) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    
    if (sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[CK SQL Error] Failed to prepare insert statement" << std::endl;
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }

    // Твой оригинальный сканирующий цикл из движка Fallout 2
    for (int type = 0; type < fallout::OBJ_TYPE_COUNT; type++) {
        int maxId = fallout::proto_max_id(type);
        if (maxId <= 1) continue;

        for (int id = 0; id < maxId; id++) {
            int pid = (type << 24) | id;
            fallout::Proto* proto = nullptr;

            if (fallout::protoGetProto(pid, &proto) != 0 || proto == nullptr) {
                continue;
            }

            // Вытаскиваем реальные данные из структур движка (сверь названия полей!)
            int fid = proto->fid; 
            
            // Генерируем имя файла прототипа (например, 00000012.pro)
            char textBuf[32];
            snprintf(textBuf, sizeof(textBuf), "%08d.pro", id);
            std::string filenameStr(textBuf);

            // Временные заглушки для имен (пока не подключили чтение текстовых .msg файлов движка)
            std::string nameStr = "Proto_" + std::to_string(type) + "_" + std::to_string(id);
            std::string descStr = "Description for proto " + std::to_string(pid);

            // Биндим данные в SQL запрос (индексы в sqlite3_bind начинаются с 1!)
            sqlite3_bind_int(stmt, 1, pid);
            sqlite3_bind_int(stmt, 2, fid);
            sqlite3_bind_int(stmt, 3, type);
            sqlite3_bind_text(stmt, 4, nameStr.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 5, filenameStr.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 6, descStr.c_str(), -1, SQLITE_TRANSIENT);

            // Выполняем шаг вставки и сбрасываем стейтмент для следующего шага
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

    // Если тип не указан (-1), ищем просто по имени, иначе фильтруем и по типу
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

