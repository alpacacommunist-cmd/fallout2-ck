#ifndef CK_PROTO_SCHEMA_H
#define CK_PROTO_SCHEMA_H

// current schema version
const int CK_PROTO_DB_VERSION = 1;

const char* const CK_PROTO_SCHEMA_SQL =
    "CREATE TABLE IF NOT EXISTS protos ("
    "pid INTEGER PRIMARY KEY, "
    "fid INTEGER, "
    "type INTEGER, "
    "name TEXT, "
    "filename TEXT, "
    "description TEXT);";

#endif
