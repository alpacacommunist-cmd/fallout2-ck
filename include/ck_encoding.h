#ifndef CK_ENCODING_H
#define CK_ENCODING_H
#include <string>
#include <string_view>

std::string system_language();
std::string utf8_to_cp1251(std::string_view utf8);
std::string utf8_to_cp1251(const std::string& utf8);
#endif
