#ifndef CK_FRM_H
#define CK_FRM_H

#include <vector>
#include <string>
#include <cstdint>

struct CkFrmFrame {
    uint16_t width;
    uint16_t height;
    int16_t offsetX;
    int16_t offsetY;
    std::vector<uint8_t> pixels; // palette indices, width * height
};

struct CkFrm {
    uint16_t fps;
    uint16_t framesPerDirection;
    std::vector<CkFrmFrame> frames[6];
    bool valid = false;
};

// path - absolute path to .frm
CkFrm ck_frm_load(const std::string& path);

#endif
