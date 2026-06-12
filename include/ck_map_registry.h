#ifndef CK_MAP_REGISTRY_H
#define CK_MAP_REGISTRY_H

#include <string>
#include <unordered_map>

struct CkMapRegistryEntry {
    int mapIdx;
    int areaIdx;
};

class CkMapRegistry {
public:
    bool load(const std::string& path);
    bool save(const std::string& path);

    CkMapRegistryEntry& resolve(const std::string& key, int nextMapIdx, int nextAreaIdx);

    bool has(const std::string& key) const;

private:
    std::unordered_map<std::string, CkMapRegistryEntry> entries;
};

#endif
