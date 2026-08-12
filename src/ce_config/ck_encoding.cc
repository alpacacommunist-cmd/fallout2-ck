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
            if ((c & 0xC0) == 0x80) {
                result += '?';
                i++;
            } else if ((c & 0xE0) == 0xC0) {
                if (i + 1 < utf8.size()) { i += 2; result += '?'; } else { result += c; i++; }
            } else if ((c & 0xF0) == 0xE0) {
                if (i + 2 < utf8.size()) { i += 3; result += '?'; } else { result += c; i++; }
            } else if ((c & 0xF8) == 0xF0) {
                if (i + 3 < utf8.size()) { i += 4; result += '?'; } else { result += c; i++; }
            } else {
                result += c;
                i++;
            }
        }
    }

    return result;
}

std::string utf8_to_cp1251(const std::string& utf8) {
    return utf8_to_cp1251(std::string_view(utf8));
}

size_t ck_cp1251_to_utf8(const char* in_cp1251, char* out_utf8, size_t max_size) {
    if (!in_cp1251 || !out_utf8 || max_size == 0) return 0;

    if (fallout::settings.system.language == "english") {
        size_t len = strlen(in_cp1251);
        if (len >= max_size) len = max_size - 1;
        memcpy(out_utf8, in_cp1251, len);
        out_utf8[len] = '\0';
        return len;
    }

    size_t out_idx = 0;
    size_t in_idx = 0;

    while (in_cp1251[in_idx] != '\0' && out_idx + 3 < max_size) {
        unsigned char c = in_cp1251[in_idx];

        if (c < 0x80) {
            out_utf8[out_idx++] = c;
        } else if (c >= 0xC0 && c <= 0xFF) { // А..я
            int code = c - 0xC0 + 0x410;
            out_utf8[out_idx++] = (char)(0xD0 + (code - 0x400) / 64);
            out_utf8[out_idx++] = (char)(0x90 + (code - 0x400) % 64);
        } else if (c == 0xA8) { // Ё
            out_utf8[out_idx++] = (char)0xD0;
            out_utf8[out_idx++] = (char)0x81;
        } else if (c == 0xB8) { // ё
            out_utf8[out_idx++] = (char)0xD1;
            out_utf8[out_idx++] = (char)0x91;
        } else {
            out_utf8[out_idx++] = c;
        }
        in_idx++;
    }

    out_utf8[out_idx] = '\0';
    return out_idx;
}
