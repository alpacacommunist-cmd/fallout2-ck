#include "geometry.h"
#include <algorithm>

HexRect geometry_build_rect_from_points(const std::vector<int>& tiles) {
    HexRect rect;
    if (tiles.size() < 4) { return rect; }

    int grid_width = fallout::tileGetHexGridWidth();
    rect.left = grid_width; rect.right = -1;
    rect.top = grid_width; rect.bottom = -1;

    for (size_t i = 0; i < 4; ++i) {
        int tile = tiles[i];
        int x = (grid_width - 1 - (tile % grid_width));
        int y = (tile / grid_width);

        if (x < rect.left)   rect.left = x; if (x > rect.right)  rect.right = x;
        if (y < rect.top)    rect.top = y; if (y > rect.bottom) rect.bottom = y;
    }

    return rect;
}

void HexRect::for_each_tile(std::function<void(int tile)> callback) const {
    if (!is_valid()) return;

    int grid_width = fallout::tileGetHexGridWidth();

    int start_x = std::max(0, left);
    int end_x = std::min(grid_width - 1, right);
    int start_y = std::max(0, top);
    int end_y = std::min(grid_width - 1, bottom);

    for (int y = start_y; y <= end_y; ++y) {
        for (int x = start_x; x <= end_x; ++x) { callback(to_tile(x, y, grid_width)); }
    }
}
