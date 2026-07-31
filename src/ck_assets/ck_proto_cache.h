#ifndef CK_PROTO_CACHE_H
#define CK_PROTO_CACHE_H

#include <string>
#include <vector>

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

    CkProtoInfo getByName(const std::string& name, int type = -1) const;

    CkProtoInfo getByPid(int pid) const;

    std::vector<CkProtoInfo> getByType(int type) const;

private:
    sqlite3* db = nullptr;

    bool buildFromEngine();
    bool createTables();
};

extern CkProtoCache gProtoCache;

#endif
