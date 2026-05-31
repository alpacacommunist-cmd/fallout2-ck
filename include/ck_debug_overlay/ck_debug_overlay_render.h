// fid, anchorTile, screenX - anchorTileX, screenY - anchorTileY
struct CkDebugHexInstance {int fid; int anchorTile; int offsetX; int offsetY; };

void blit_debug_hex_colored(
		const unsigned char* src, int width, int height, int srcPitch,
		unsigned char* dest, int destX, int destY, int destPitch,
		unsigned char edgeColor, unsigned char innerColor
);

void ck_debug_overlay_clear();
void ck_debug_overlay_add_hex(int fid, int anchorTile, int offsetX, int offsetY);
void ck_debug_overlay_persistent_hexes(fallout::Rect* rect);

int ck_debug_overlay_build_interface_fid(int fid);
