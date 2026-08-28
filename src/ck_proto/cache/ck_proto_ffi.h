#ifndef CK_PROTO_FFI_H
#define CK_PROTO_FFI_H

#include "ck_api.h"

struct CkProtoInfoFFI {
    int pid;
    int fid;
    int type;
    int sid;

    const char* name;
    const char* filename;
    const char* description;
};

CK_API bool ck_proto_get_by_pid(int pid, CkProtoInfoFFI* out_info);
CK_API bool ck_proto_get_by_name(const char* name, int type, CkProtoInfoFFI* out_info);

CK_API int ck_proto_get_by_type(int type, CkProtoInfoFFI* out_array, int max_count);

#endif
