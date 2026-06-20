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
            // Обработка символов из латиницы-1 / знаков препинания (2 байта)
            unsigned char c2 = utf8[i + 1];
            if (c2 == 0xAB) {
                result += (char)0xAB; // « (Левая кавычка в CP1251)
            } else if (c2 == 0xBB) {
                result += (char)0xBB; // » (Правая кавычка в CP1251)
            } else if (c2 == 0xA0) {
                result += (char)0x20; // Неразрывный пробел -> обычный пробел
            } else {
                result += '?';
            }
            i += 2;
        } else if (c == 0xE2 && i + 2 < utf8.size() && utf8[i + 1] == 0x80) {
            // Обработка популярных 3-байтовых символов пунктуации (тире, многоточие)
            unsigned char c3 = utf8[i + 2];
            if (c3 == 0x94 || c3 == 0x93) {
                result += (char)0x96; // Длинное/среднее тире — -> в CP1251 тире (0x96)
            } else if (c3 == 0xA6) {
                result += "...";       // Многоточие … -> превращаем в три точки
            } else {
                result += '?';
            }
            i += 3;
        } else {
            // Безопасный пропуск всех остальных неизвестных многобайтовых UTF-8 символов
            if ((c & 0xE0) == 0xC0)      i += 2;
            else if ((c & 0xF0) == 0xE0) i += 3;
            else if ((c & 0xF8) == 0xF0) i += 4;
            else                          i += 1;

            result += '?';
        }
    }

    return result;
}
