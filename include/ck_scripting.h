#ifndef CK_SCRIPTING_H
#define CK_SCRIPTING_H

void ckScriptingInit();
void ckScriptingExit();

void ckHookOnGameStart();
void ckHookOnDayPassed();
void ckHookOnGameLoaded();
void ckHookOnMapEnter();

void ckHookOnAfterRest(int hours, int minutes);
void ckHookOnTimeAdvance(int hours, int minutes);

int ckGetConfigInt(const char* key, int default_value);

#endif // CK_SCRIPTING_H
