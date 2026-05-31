#include "ck_debug_overlay/ck_debug_overlay.h"
#include "ck_debug_overlay/ck_debug_overlay_render.h"

#include "tile.h"
#include "object.h"
#include "mouse.h"
#include "debug.h"

void palette_paint_hover(int anchorTile) {
    if (!ck_debug_overlay_enabled()) return;

    static int lastTile = -1;
    static unsigned char currentColor = 0;

    int mouseX, mouseY;
    fallout::mouseGetPosition(&mouseX, &mouseY);

    int tile = fallout::tileFromScreenXY(mouseX, mouseY, fallout::gElevation);

    if (tile == lastTile) return;

    lastTile = tile;

    int anchorScreenX, anchorScreenY;

    fallout::tileToScreenXY(anchorTile, &anchorScreenX, &anchorScreenY);

    int screenX, screenY;

    fallout::tileToScreenXY(tile, &screenX, &screenY);

    fallout::debugPrint("[CK] color=%d tile=%d\n", currentColor, tile);

    ck_debug_overlay_add_hex(
        999,
        anchorTile,
        screenX - anchorScreenX,
        screenY - anchorScreenY,
        { currentColor, currentColor }
    );

    currentColor++;
}
