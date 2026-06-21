#ifndef CK_ENCODING_H
#define CK_ENCODING_H
#include <string>
#include "settings.h"
std::string system_language();
std::string utf8_to_cp1251(const std::string& utf8);
#endif
