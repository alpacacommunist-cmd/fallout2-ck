#ifndef CK_GEOMETRY_H
#define CK_GEOMETRY_H

#include <vector>
#include <functional>

namespace fallout {
    int tileGetHexGridWidth();
}

struct HexRect {
    int left = 0;
    int right = -1;
    int top = 0;
    int bottom = -1;

    bool is_valid() const { return left <= right && top <= bottom; }

    static int to_tile(int x, int y, int grid_width) {
        return y * grid_width + (grid_width - 1 - x);
    }

    void for_each_tile(std::function<void(int tile)> callback) const;
};

HexRect geometry_build_rect_from_points(const std::vector<int>& tiles);

#endif
