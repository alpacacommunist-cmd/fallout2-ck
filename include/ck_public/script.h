#ifndef CK_PUBLIC_SCRIPT_H
#define CK_PUBLIC_SCRIPT_H

namespace fallout {
    struct Script;
}

namespace ck::script {
    void handle_global_proc_event(int sid, int proc);
	bool try_handle(int sid, int proc);
	bool owns_sid(int sid);

	fallout::Script* get_dummy(int sid);
}

#endif
