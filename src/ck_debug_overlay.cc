#include "ck_debug_overlay.h"
#include "ck_rendering.h"

#include <cstdlib>

#include "color.h"
#include "mouse.h"
#include "tile.h"
#include "object.h"
#include "light.h"
#include "art.h"
#include "draw.h"
#include "display_monitor.h"
#include "debug.h"


static void draw_misc_art(int fid, int x, int y, fallout::Rect* rect);
static void blit_debug_hex_colored(
    const unsigned char* src, int width, int height, int srcPitch,
    unsigned char* dest, int destX, int destY, int destPitch,
    unsigned char edgeColor, unsigned char innerColor
) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char pixel = src[y * srcPitch + x];

            // 0x00 - прозрачный фон, его всегда пропускаем
            if (pixel == 0) continue;

            int outX = destX + x;
            int outY = destY + y;
            unsigned char* destPixel = dest + (outY * destPitch) + outX;

            // Если оригинальный пиксель достаточно яркий (граница хекса),
            // красим его основным ярким цветом
            if (pixel > 150) {
                *destPixel = edgeColor;
            } else {
                // Если это внутренняя полупрозрачная тень,
                // заливаем её приглушенным оттенком того же цвета для объема
                *destPixel = innerColor;
                
                // АЛЬТЕРНАТИВА: Если ты хочешь, чтобы хекс внутри был 100% ПУСТЫМ,
                // просто закомментируй строчку выше (*destPixel = innerColor;).
            }
        }
    }
}


// fid, anchorTile, screenX - anchorTileX, screenY - anchorTileY
struct CkDebugHexInstance {int fid; int anchorTile; int offsetX; int offsetY; };
static std::vector<CkDebugHexInstance> gPersistentHexes;

static int ck_debug_overlay_build_interface_fid(int fid) {
    return fallout::buildFid(fallout::OBJ_TYPE_INTERFACE, fid, 0, 0, 0);
}

void ck_debug_overlay_add_hex(int fid, int anchorTile, int offsetX, int offsetY) {
    gPersistentHexes.push_back({ fid, anchorTile, offsetX, offsetY });
}

static void ck_debug_overlay_persistent_hexes(fallout::Rect* rect) {
	for (const auto& hex : gPersistentHexes) {
		int screenX, screenY;
		fallout::tileToScreenXY(hex.anchorTile, &screenX, &screenY);

		int fid = ck_debug_overlay_build_interface_fid(hex.fid);

		draw_misc_art(fid, screenX + hex.offsetX, screenY + hex.offsetY, rect);
	}
}

static void draw_misc_art(int fid, int x, int y, fallout::Rect* rect) {
	fallout::CacheEntry* cacheEntry;
	fallout::Art* art = artLock(fid, &cacheEntry);

    if (art == nullptr) return;

    int width = artGetWidth(art, 0, 0);
    int height = artGetHeight(art, 0, 0);

	fallout::Rect artRect;
    artRect.left = x, artRect.top = y, artRect.right = x + width - 1, artRect.bottom = y + height - 1;

	fallout::Rect intersection;
    if (fallout::rectIntersection(&artRect, rect, &intersection) == -1) {
		fallout::artUnlock(cacheEntry);
        return;
    }

    unsigned char* src = fallout::artGetFrameData(art, 0, 0);

    // Сдвигаем указатель источника с учетом пересечения
    src += width * (intersection.top - y) + (intersection.left - x);

    // Вытаскиваем чистый ID из FID, чтобы узнать, какой цвет от нас хотят
    int artId = fid & 0xFFFF;

	   
    // Дефолтные цвета (Зеленый)
    unsigned char edgeColor = 0xFE; 
    unsigned char innerColor = 0x9E;
    
	// Мапим ID (996 - 999) на пары цветов палитры Fallout 2
	if (artId == 998) { 
		edgeColor = 0xFD; // Ярко-красный
		innerColor = 0x89; // Темно-красный
	} 
	else if (artId == 997) { 
		edgeColor = 0xE4; // Ярко-синий
		innerColor = 0xCD; // Темно-синий
	} 
	else if (artId == 996) { 
		edgeColor = 0xBC; // Ярко-желтый
		innerColor = 0xB3; // Темно-желтый
	}

	// Вызываем блиттер с палитрой цветов
	blit_debug_hex_colored(
			src, 
			rectGetWidth(&intersection), 
			rectGetHeight(&intersection),
			width, 
			fallout::tileGetWindowBuffer(), 
			intersection.left, 
			intersection.top,
			fallout::tileGetWindowPitch(), 
			edgeColor,
			innerColor
	);

    // Разблокируем только ПОСЛЕ отрисовки, так как мы читали напрямую из src!
    artUnlock(cacheEntry);
}

static bool gDebugOverlayEnabled = false;


bool ck_debug_overlay_enabled() {
    return gDebugOverlayEnabled;
}

void ck_debug_overlay_clear() {
	gPersistentHexes.clear();
}

void ck_debug_overlay_toggle() {
    gDebugOverlayEnabled = !gDebugOverlayEnabled;

	if (gDebugOverlayEnabled) {
		fallout::displayMonitorAddMessage("[CK] Debug Overlay: ON");
	} else {
		fallout::displayMonitorAddMessage("[CK] Debug Overlay: OFF");
		ck_debug_overlay_clear();

		fallout::tileWindowRefresh();
	}
}

//// WORKING VERSION
///

void working_hover(int anchorTile) {
    if (!gDebugOverlayEnabled) return; 

    static int lastTile = -1;

    int anchorScreenX, anchorScreenY;
    fallout::tileToScreenXY(anchorTile, &anchorScreenX, &anchorScreenY);

    int mouseX, mouseY;
    fallout::mouseGetPosition(&mouseX, &mouseY);

	int screenX, screenY;
    int tile = fallout::tileFromScreenXY(mouseX, mouseY, fallout::gElevation);

    if (tile == lastTile) return;

    lastTile = tile;

    int gridWidth = fallout::tileGetHexGridWidth();
    int tileX = gridWidth - 1 - tile % gridWidth,
		tileY = tile / gridWidth;

	fallout::Object* blocker = fallout::_obj_blocking_at(nullptr, tile, fallout::gElevation);
	bool blocked = blocker != nullptr;

	fallout::tileToScreenXY(tile, &screenX, &screenY);

	fallout::debugPrint(
			"[CK] Hover tile=%d hex=(%d,%d) screen=(%d, %d) blocked=%s\n", tile, tileX, tileY,
			screenX - anchorScreenX, screenY - anchorScreenY,
			blocked ? "true" : "false");

	ck_debug_overlay_add_hex(997, anchorTile, screenX - anchorScreenX, screenY - anchorScreenY);
}

void ck_debug_overlay_render(fallout::Rect* rect) {
    if (!gDebugOverlayEnabled) return;

	ck_debug_overlay_persistent_hexes(rect);
	if ((fallout::mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) != 0) {
		working_hover(17290);
	}
}

