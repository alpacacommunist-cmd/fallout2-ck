#ifndef CK_ENCODING_H
#define CK_ENCODING_H
#include <string>
#include <string_view>
#include <ck_api.h>

std::string system_language();
std::string utf8_to_cp1251(std::string_view utf8);
std::string utf8_to_cp1251(const std::string& utf8);

CK_API size_t ck_cp1251_to_utf8(const char* in_cp1251, char* out_utf8, size_t max_size);
#endif
