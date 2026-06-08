#ifndef CK_PROTO_CACHE_H
#define CK_PROTO_CACHE_H

#include <string>
#include <unordered_map>
#include <vector>

struct CkProtoInfo {
    int pid = -1;
    int fid = -1;
    std::string name;
	std::string filename;
    int type = -1;  // OBJ_TYPE_*
};

class CkProtoCache {
public:
    // Initialize cache from JSON file, or rebuild from engine if doesn't exist
    bool initialize(const std::string& cachePath);

    // Look up proto by name (e.g., "tree10")
    // Returns nullptr if not found
    const CkProtoInfo* getByName(const std::string& name, int type = -1) const;

    // Look up proto by PID
    const CkProtoInfo* getByPid(int pid) const;

    // Get all protos of a type
    std::vector<CkProtoInfo> getByType(int type) const;

private:
    std::unordered_map<std::string, CkProtoInfo> nameIndex;  // "tree10" → CkProtoInfo
    std::unordered_map<int, CkProtoInfo> pidIndex;           // pid → CkProtoInfo

    bool loadFromJson(const std::string& path);
    bool buildFromEngine(const std::string& cachePath);
};

extern CkProtoCache gProtoCache;
#endif
