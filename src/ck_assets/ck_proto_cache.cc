#include "ck_assets/ck_proto_cache.h"
#include "proto.h"
#include "obj_types.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

bool CkProtoCache::initialize(const std::string& cachePath) {
    // Try loading from JSON first
    if (loadFromJson(cachePath)) {
        std::cout << "[CK Proto Cache] Loaded " << nameIndex.size() << " protos from " << cachePath << std::endl;
        return true;
    }

    // If that fails, rebuild from engine
    std::cout << "[CK Proto Cache] Building proto cache from engine..." << std::endl;
    return buildFromEngine(cachePath);
}

bool CkProtoCache::loadFromJson(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines, array delimiters
        if (line.empty() || line[0] == '[' || line[0] == ']') continue;

        // Remove trailing comma and whitespace
        while (!line.empty() && (line.back() == ',' || line.back() == ' ' || line.back() == '\n')) {
            line.pop_back();
        }
        if (line.empty()) continue;

        try {
            CkProtoInfo info;

            // Ultra-simple JSON parsing: {"name":"...","pid":...,"fid":...,"type":...}
            // Extract name
            size_t nameStart = line.find("\"name\":\"");
            if (nameStart == std::string::npos) continue;
            nameStart += 8;
            size_t nameEnd = line.find("\"", nameStart);
            if (nameEnd == std::string::npos) continue;
            info.name = line.substr(nameStart, nameEnd - nameStart);

            // Extract pid
            size_t pidStart = line.find("\"pid\":");
            if (pidStart == std::string::npos) continue;
            pidStart += 6;
            size_t pidEnd = line.find(",", pidStart);
            if (pidEnd == std::string::npos) pidEnd = line.find("}", pidStart);
            info.pid = std::stoi(line.substr(pidStart, pidEnd - pidStart));

            // Extract fid
            size_t fidStart = line.find("\"fid\":");
            if (fidStart == std::string::npos) continue;
            fidStart += 6;
            size_t fidEnd = line.find(",", fidStart);
            if (fidEnd == std::string::npos) fidEnd = line.find("}", fidStart);
            info.fid = std::stoi(line.substr(fidStart, fidEnd - fidStart));

            // Extract type
            size_t typeStart = line.find("\"type\":");
            if (typeStart == std::string::npos) continue;
            typeStart += 7;
            size_t typeEnd = line.find(",", typeStart);
            if (typeEnd == std::string::npos) typeEnd = line.find("}", typeStart);
            info.type = std::stoi(line.substr(typeStart, typeEnd - typeStart));

            nameIndex[info.name] = info;
            pidIndex[info.pid] = info;
        } catch (const std::exception& e) {
            // Skip malformed lines
            continue;
        }
    }

    return !nameIndex.empty();
}

bool CkProtoCache::buildFromEngine(const std::string& cachePath) {
    // Scan all engine protos
    for (int type = 0; type < fallout::OBJ_TYPE_COUNT; type++) {
        int maxId = fallout::proto_max_id(type);

        for (int id = 0; id < maxId; id++) {
            int pid = (type << 24) | id;
            fallout::Proto* proto = nullptr;

            if (fallout::protoGetProto(pid, &proto) == 0 && proto != nullptr) {
                const char* protoName = fallout::protoGetName(pid);
                if (protoName == nullptr || strlen(protoName) == 0) continue;

                CkProtoInfo info;
                info.pid = pid;
                info.fid = proto->fid;
                info.type = type;
                info.name = std::string(protoName);

                nameIndex[info.name] = info;
                pidIndex[info.pid] = info;
            }
        }
    }

    std::cout << "[CK Proto Cache] Scanned " << nameIndex.size() << " protos" << std::endl;

    // Write to JSON
    fs::path jsonPath(cachePath);
    fs::create_directories(jsonPath.parent_path());

    std::ofstream file(cachePath);
    if (!file.is_open()) {
        std::cerr << "[CK Proto Cache] Failed to write " << cachePath << std::endl;
        return false;
    }

    file << "[\n";
    bool first = true;
    for (const auto& [name, info] : nameIndex) {
        if (!first) file << ",\n";
        file << "  {\"name\":\"" << name << "\",\"pid\":" << info.pid
             << ",\"fid\":" << info.fid << ",\"type\":" << info.type << "}";
        first = false;
    }
    file << "\n]\n";
    file.close();

    std::cout << "[CK Proto Cache] Wrote cache to " << cachePath << std::endl;
    return !nameIndex.empty();
}

const CkProtoInfo* CkProtoCache::getByName(const std::string& name, int type) const {
    auto it = nameIndex.find(name);
    if (it == nameIndex.end()) return nullptr;

    // If type filter is specified, check it matches
    if (type != -1 && it->second.type != type) return nullptr;

    return &it->second;
}

const CkProtoInfo* CkProtoCache::getByPid(int pid) const {
    auto it = pidIndex.find(pid);
    return (it != pidIndex.end()) ? &it->second : nullptr;
}

std::vector<CkProtoInfo> CkProtoCache::getByType(int type) const {
    std::vector<CkProtoInfo> result;
    for (const auto& [name, info] : nameIndex) {
        if (info.type == type) {
            result.push_back(info);
        }
    }
    return result;
}
