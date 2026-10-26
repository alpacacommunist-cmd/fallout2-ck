#ifndef CK_PUBLIC_SCRIPT_H
#define CK_PUBLIC_SCRIPT_H

namespace fallout {
    struct Script;
}

namespace ck::script {
    void handle_global_proc_event(int sid, int proc);
    bool handle_proc(fallout::Script* script, int proc);
    bool handle_proc(int sid, int proc);
}

#endif
