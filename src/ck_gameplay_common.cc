#include <cstring>

#include "engine/config/encoding.h"
#include "utils/fallout/types.h"

#include "obj_types.h"
#include "geometry.h"
#include "map_defs.h"

namespace f_types = ck::utils::types;
namespace fallout {
    // loadsave.cc
    bool _isLoadingGame();
    int mapIdBeingLoaded();
    // game.cc
	void displayMonitorAddMessage(const char* str);
    // game_sound.cc
    int soundPlayFile(const char* name);
    // tile.cc
    void tileWindowRefreshRect(Rect* rect, MapElevation elevation);

    enum Color : unsigned char;
    enum CombatState : unsigned int {
        COMBAT_STATE_OUT_COMBAT = 0x00, COMBAT_STATE_IN_COMBAT = 0x01,
        COMBAT_STATE_PLAYER_TURN = 0x02, COMBAT_STATE_EXIT_REQUESTED = 0x08,
    };

    extern CombatState gCombatState;

    extern int gElevation;
    // map.cc
    Map mapGetCurrentMap();
    struct TileData { int field_0[SQUARE_GRID_SIZE]; };
    extern TileData* _square[ELEVATION_COUNT];
    // text_object.cc
    int textObjectAdd(Object* object, char* string, int font, ColorWithFlags color, ColorWithFlags outlineColor, Rect* rect);

    inline int textObjectAdd(Object* object, char* string, int font, Color color, Color outlineColor, Rect* rect)
    {
        return textObjectAdd(object, string, font, color | DRAW_TEXT_FLAG_NONE, outlineColor | DRAW_TEXT_FLAG_NONE, rect);
    }
}

namespace ck::common {
    unsigned int current_combat_state() {
        return fallout::gCombatState;
    }

    bool in_combat() {
        return (current_combat_state() & fallout::COMBAT_STATE_IN_COMBAT) != 0;
    }

    int current_elevation() {
        return fallout::gElevation;
    }

    int current_map_id() {
        return static_cast<int>(fallout::mapGetCurrentMap());
    }

    bool game_is_loading() {
        return fallout::_isLoadingGame();
    }

    int loading_map_id() {
        return fallout::mapIdBeingLoaded();
    }

    int square(int elevation, int square_tile) {
        if (fallout::_square[elevation] == nullptr) return -1;

        return fallout::_square[elevation]->field_0[square_tile];
    }

    bool hex_grid_tile_is_valid(int tile) {
        return tile >= 0 && tile < HEX_GRID_SIZE;
    }

    bool square_grid_tile_is_valid(int tile) {
        return tile >= 0 && tile < SQUARE_GRID_SIZE;
    }

    void play_sfx(const char *name) {
        if (name != nullptr) fallout::soundPlayFile(name);
    }

    void print_monitor_message(const char* message) {
        std::string converted = utf8_to_cp1251(std::string_view(message));
        fallout::displayMonitorAddMessage(converted.c_str());
    }

    bool object_float_msg(void* ptr, const char* text, int msg_type) {
        if (!ptr) return false; auto* object = static_cast<fallout::Object*>(ptr);
        if (!object) return false;
        if (object->elevation != fallout::gElevation) return false;

        int color = fallout::_colorTable[32747], background_color = fallout::_colorTable[0], font = 101;

        switch (msg_type) {
            case 1: // (FLOATING_MESSAGE_TYPE_WHITE)
                color = fallout::_colorTable[32767];
                break;
            case 2: // (FLOATING_MESSAGE_TYPE_RED)
                color = fallout::_colorTable[31744];
                break;
            case 3: // (FLOATING_MESSAGE_TYPE_GREEN)
                color = fallout::_colorTable[992];
                break;
            case 4: // (FLOATING_MESSAGE_TYPE_BLUE)
                color = fallout::_colorTable[31];
                break;
            default: // YELLOW
                color = fallout::_colorTable[32747];
                break;
        }

        fallout::Rect rect;
        std::string converted = utf8_to_cp1251(std::string_view(text));

        std::vector<char> safe_buffer(converted.size() + 5, '\0');
        std::memcpy(safe_buffer.data(), converted.c_str(), converted.size());

        char* safe_text_ptr = safe_buffer.data();

        if (fallout::textObjectAdd(object, safe_text_ptr, font,
                    static_cast<fallout::Color>(color), static_cast<fallout::Color>(background_color), &rect) != -1) {
            fallout::tileWindowRefreshRect(&rect, f_types::map_elevation(object->elevation));

            return true;
        }

        return false;
    }
}

