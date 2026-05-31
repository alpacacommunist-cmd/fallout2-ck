// fid, anchorTile, screenX - anchorTileX, screenY - anchorTileY

struct DebugHexColor { unsigned char edge; unsigned char inner; };
struct CkDebugHexInstance {int artId; int anchorTile; int offsetX; int offsetY; DebugHexColor color; };

constexpr DebugHexColor RED { 135, 135 };
constexpr DebugHexColor GREEN { 198, 198 };
constexpr DebugHexColor BLUE { 105, 105 };
constexpr DebugHexColor YELLOW { 57, 57 };

void blit_debug_hex_colored(
		const unsigned char* src, int width, int height, int srcPitch,
		unsigned char* dest, int destX, int destY, int destPitch,
		unsigned char edgeColor, unsigned char innerColor
);

void ck_debug_overlay_clear();
void ck_debug_overlay_shutdown(); // clears cached art
void ck_debug_overlay_add_hex(int fid, int anchorTile, int offsetX, int offsetY, DebugHexColor color);
void ck_debug_overlay_persistent_hexes(fallout::Rect* rect);

int ck_debug_overlay_build_interface_fid(int fid);
