#pragma once

namespace ck::mods {
    const std::vector<const char*>& all();
    const char* ptr(const char* mod_id);

    bool reload();
    bool reloading();

    bool add(const char* mod_id);
    bool drop(const char* mod_id);
}
