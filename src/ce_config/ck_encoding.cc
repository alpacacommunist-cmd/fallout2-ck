#include "ck_encoding.h"
#include "settings.h"

std::string system_language() { return fallout::settings.system.language; }

std::string utf8_to_cp1251(std::string_view utf8) {
    if (system_language() == "english") return std::string(utf8);

    std::string result;
    result.reserve(utf8.size());

    size_t i = 0;
    while (i < utf8.size()) {
        unsigned char c = utf8[i];

        if (c < 0x80) {
            result += c;
            i++;
        } else if (c == 0xD0 && i + 1 < utf8.size()) {
            unsigned char c2 = utf8[i + 1];
            if (c2 == 0x81) {
                result += (char)0xA8; // Ё
            } else if (c2 >= 0x90 && c2 <= 0xBF) {
                result += (char)(c2 - 0x90 + 0xC0); // А..я
            } else {
                result += '?';
            }
            i += 2;
        } else if (c == 0xD1 && i + 1 < utf8.size()) {
            unsigned char c2 = utf8[i + 1];
            if (c2 == 0x91) {
                result += (char)0xB8; // ё
            } else if (c2 >= 0x80 && c2 <= 0x8F) {
                result += (char)(c2 - 0x80 + 0xF0); // р..я
            } else {
                result += '?';
            }
            i += 2;
        } else if (c == 0xC2 && i + 1 < utf8.size()) {
            unsigned char c2 = utf8[i + 1];
            if (c2 == 0x4B || c2 == 0xAB) { // подстраховка для кавычек
                result += (char)0xAB;
            } else if (c2 == 0xBB) {
                result += (char)0xBB;
            } else if (c2 == 0xA0) {
                result += (char)0x20;
            } else {
                result += '?';
            }
            i += 2;
        } else if (c == 0xE2 && i + 2 < utf8.size() && utf8[i + 1] == 0x80) {
            unsigned char c3 = utf8[i + 2];
            if (c3 == 0x94 || c3 == 0x93) {
                result += (char)0x96;
            } else if (c3 == 0xA6) {
                result += "...";
            } else {
                result += '?';
            }
            i += 3;
        } else {
            if ((c & 0xE0) == 0xC0)      i += 2;
            else if ((c & 0xF0) == 0xE0) i += 3;
            else if ((c & 0xF8) == 0xF0) i += 4;
            else                          i += 1;

            result += '?';
        }
    }

    return result;
}

std::string utf8_to_cp1251(const std::string& utf8) {
    return utf8_to_cp1251(std::string_view(utf8));
}
