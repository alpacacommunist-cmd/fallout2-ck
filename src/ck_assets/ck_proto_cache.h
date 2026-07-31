#ifndef CK_PROTO_CACHE_H
#define CK_PROTO_CACHE_H

#include <string>

struct CkProtoInfo {
    int pid = -1;
    int fid = -1;
    std::string name;
    std::string filename;
    std::string description;
    int type = -1;
};

struct sqlite3;

class CkProtoCache {
public:
    CkProtoCache();
    ~CkProtoCache();

    bool initialize(const std::string& cachePath);
    struct sqlite3* get_db_handle() const { return db; }

private:
    sqlite3* db = nullptr;

    bool buildFromEngine();
    bool createTables();
};

extern CkProtoCache gProtoCache;

#endif
