#ifndef CK_PUBLIC_HOOKS_H
#define CK_PUBLIC_HOOKS_H

namespace ck::hooks::system {
    void init(int argc, char** argv);
    void engine_ready();
    void scripts_reset();
    void exit();
}

namespace ck::mods {
    bool reload();
}

#endif
