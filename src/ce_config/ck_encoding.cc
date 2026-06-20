// src/ce_config/ck_encoding.cc
#include "ck_encoding.h"

// UTF-8 кириллица -> cp1251
// У-8 кириллица начинается с 0xD0/0xD1
// А(0x410)..Я(0x42F) -> cp1251 0xC0..0xDF
// а(0x430)..я(0x44F) -> cp1251 0xE0..0xFF
// Ё(0x401) -> 0xA8, ё(0x451) -> 0xB8

std::string utf8_to_cp1251(const std::string& utf8) {
    std::string result;
    result.reserve(utf8.size());

    size_t i = 0;
    while (i < utf8.size()) {
        unsigned char c = utf8[i];

        if (c < 0x80) {
            // ASCII
            result += c;
            i++;
        } else if (c == 0xD0 && i + 1 < utf8.size()) {
            unsigned char c2 = utf8[i + 1];

            if (c2 == 0x81) {
                result += (char)0xA8; // Ё
            } else if (c2 >= 0x90 && c2 <= 0xBF) {
                // А(0x90)..я(0xBF) -> cp1251 0xC0..0xEF
                result += (char)(c2 - 0x90 + 0xC0);
            } else {
                result += '?'; // unknown symbol
            }

            i += 2;
        } else if (c == 0xD1 && i + 1 < utf8.size()) {
            unsigned char c2 = utf8[i + 1];

            if (c2 == 0x91) {
                result += (char)0xB8; // ё
            } else if (c2 >= 0x80 && c2 <= 0x8F) {
                // р(0x80)..я(0x8F) -> cp1251 0xF0..0xFF
                result += (char)(c2 - 0x80 + 0xF0);
            } else {
                result += '?';
            }

            i += 2;
        } else {
            // skip other multibyte symbols
            // first bite is length
            if ((c & 0xE0) == 0xC0)      i += 2;
            else if ((c & 0xF0) == 0xE0) i += 3;
            else if ((c & 0xF8) == 0xF0) i += 4;
            else                          i += 1;

            result += '?';
        }
    }

    return result;
}
