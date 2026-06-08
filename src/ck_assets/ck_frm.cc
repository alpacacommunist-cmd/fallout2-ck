#include "ck_assets/ck_frm.h"
#include <cstdio>
#include <cstring>
#include <iostream>

template<typename T>
static bool read(FILE* f, T& value) {
    return fread(&value, sizeof(T), 1, f) == 1;
}

static uint16_t swap16(uint16_t v) {
    return (v >> 8) | (v << 8);
}

static uint32_t swap32(uint32_t v) {
    return ((v & 0xFF000000) >> 24) |
           ((v & 0x00FF0000) >> 8)  |
           ((v & 0x0000FF00) << 8)  |
           ((v & 0x000000FF) << 24);
}

static int16_t swap16s(int16_t v) {
    return (int16_t)swap16((uint16_t)v);
}

CkFrm ck_frm_load(const std::string& path) {
    CkFrm result;

    FILE* f = fopen(path.c_str(), "rb");
    if (f == nullptr) {
        std::cerr << "[CK FRM] Cannot open: " << path << std::endl;
        return result; // result.valid == false
    }

    // --- header ---
    uint32_t version;
    uint16_t fps, actionFrame, framesPerDirection;
    int16_t  shiftX[6], shiftY[6];
    uint32_t frameOffset[6];
    uint32_t dataSize;

    if (!read(f, version))           { fclose(f); return result; }
    if (!read(f, fps))               { fclose(f); return result; }
    if (!read(f, actionFrame))       { fclose(f); return result; }
    if (!read(f, framesPerDirection)){ fclose(f); return result; }

    for (int i = 0; i < 6; i++) {
        if (!read(f, shiftX[i])) { fclose(f); return result; }
    }
    for (int i = 0; i < 6; i++) {
        if (!read(f, shiftY[i])) { fclose(f); return result; }
    }
    for (int i = 0; i < 6; i++) {
        if (!read(f, frameOffset[i])) { fclose(f); return result; }
    }
    if (!read(f, dataSize)) { fclose(f); return result; }

    // swap endianness
    fps               = swap16(fps);
    framesPerDirection = swap16(framesPerDirection);
    for (int i = 0; i < 6; i++) {
        shiftX[i]      = swap16s(shiftX[i]);
        shiftY[i]      = swap16s(shiftY[i]);
        frameOffset[i] = swap32(frameOffset[i]);
    }

    result.fps = fps;
    result.framesPerDirection = framesPerDirection;

    // --- read direction 0 ---
    // header ends at 62 (0x3E)
    // direction 0 starts right after
    uint32_t dir0Offset = 62 + frameOffset[0];
    if (fseek(f, dir0Offset, SEEK_SET) != 0) {
        std::cerr << "[CK FRM] Seek failed: " << path << std::endl;
        fclose(f);
        return result;
    }

    // direction 0 all frames
    for (int i = 0; i < framesPerDirection; i++) {
        uint16_t width, height;
        uint32_t size;
        int16_t  offsetX, offsetY;

        if (!read(f, width))   break;
        if (!read(f, height))  break;
        if (!read(f, size))    break;
        if (!read(f, offsetX)) break;
        if (!read(f, offsetY)) break;

        width   = swap16(width);
        height  = swap16(height);
        size    = swap32(size);
        offsetX = swap16s(offsetX);
        offsetY = swap16s(offsetY);

        CkFrmFrame frame;
        frame.width   = width;
        frame.height  = height;
        frame.offsetX = offsetX;
        frame.offsetY = offsetY;
        frame.pixels.resize(size);

        if (fread(frame.pixels.data(), 1, size, f) != size) {
            std::cerr << "[CK FRM] Pixel read failed at frame " << i << std::endl;
            break;
        }

        result.frames.push_back(std::move(frame));
    }

    fclose(f);

    if (!result.frames.empty()) {
        result.valid = true;
        std::cout << "[CK FRM] Loaded: " << path
                  << " frames=" << result.frames.size()
                  << " size=" << result.frames[0].width << "x" << result.frames[0].height
                  << std::endl;
    }

    return result;
}
