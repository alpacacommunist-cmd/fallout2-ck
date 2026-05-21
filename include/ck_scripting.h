#ifndef CK_SCRIPTING_H
#define CK_SCRIPTING_H

void ckScriptingInit();
void ckScriptingExit();

void ckHookOnGameStart();

int ckGetConfigInt(const char* key, int default_value);

#endif // CK_SCRIPTING_H
