#include "debug/overlay.h"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_sdl2.h"
#include "vendor/imgui/backends/imgui_impl_sdlrenderer2.h"

#include "fallout2-ce/art.h"
#include "fallout2-ce/svga.h"

#include "log.h"
static logging::Context logger{"debug/ck_ui.cc", logging::Source::Native};

namespace fallout {
    extern int gElevation;

    extern SDL_Window* gSdlWindow;
    extern SDL_Renderer* gSdlRenderer;

    unsigned char* directDrawGetPalette();

	void mouseGetPosition(int* out_x, int* out_y);
	int tileFromScreenXY(int x, int y, bool ignoreBounds);
    int tileToScreenXY(int tile, int* screenX, int* screenY);
}

static bool g_initialized = false;
static bool g_show_overlay = false;

static int g_brush_fid = -1;
static SDL_Texture* g_brush_texture = nullptr;

SDL_Texture* create_sdl_texture_from_fid(SDL_Renderer* renderer, int fid) {
    fallout::CacheEntry* entry = nullptr;
    fallout::Art* art = fallout::artLock(fallout::FrmId(fid), &entry);

    if (!entry) return nullptr;
    int art_height = fallout::artGetHeight(art, 0, static_cast<fallout::Rotation>(0));
    int art_width = fallout::artGetWidth(art, 0, static_cast<fallout::Rotation>(0));

    // frm to sdl surface
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(
        fallout::artGetFrameData(art, 0, static_cast<fallout::Rotation>(0)),
        art_width,
        art_height,
        8, // 8 bit per pixel (index)
        art_width,
        SDL_PIXELFORMAT_INDEX8
    );

    if (!surface) return nullptr;

    unsigned char* current_palette = fallout::directDrawGetPalette();

    // sdl palette << 2 shift
    SDL_Palette* pool_palette = SDL_AllocPalette(256);
    SDL_Color colors[256];

    for (int index = 0; index < 256; index++) {
        colors[index].r = current_palette[index * 3] << 2;
        colors[index].g = current_palette[index * 3 + 1] << 2;
        colors[index].b = current_palette[index * 3 + 2] << 2;

        // trans backround
        colors[index].a = (index == 0) ? 0 : 255;
    }

    SDL_SetPaletteColors(pool_palette, colors, 0, 256);
    SDL_SetSurfacePalette(surface, pool_palette);

    SDL_SetColorKey(surface, SDL_TRUE, 0);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_FreeSurface(surface);
    SDL_FreePalette(pool_palette);

    return texture;
}

namespace ck::debug::ui {
    static bool g_initialized = false;

    void initialize() {
        if (g_initialized) return;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplSDL2_InitForSDLRenderer(fallout::gSdlWindow, fallout::gSdlRenderer);
        ImGui_ImplSDLRenderer2_Init(fallout::gSdlRenderer);

        g_initialized = true;
    }

    void shutdown() {
        if (!g_initialized) return;
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        g_initialized = false;
    }

    void process_event(SDL_Event *event) {
        if (!g_initialized) return;
        if (ck::debug::enabled()) { 
            ImGui_ImplSDL2_ProcessEvent(event); 
        }
    }

    void render(SDL_Renderer *renderer) {
        if (!g_initialized || !ck::debug::enabled()) return;

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        bool show_window = true;
        ImGui::Begin("Fallout 2 CK - Asset Manager", &show_window);
        ImGui::Text("Hello from LuaJIT runtime workspace!");

        if (!show_window) {
            ck::debug::toggle();
        }

        if (ImGui::Button("Spawn Scenery (Tree)")) {
            logger.info.banner("BRUSH ACTIVATED!");

            if (g_brush_texture) { SDL_DestroyTexture(g_brush_texture); }

            // g_brush_fid = 33555377;
            g_brush_fid = 0x040005E4;
            g_brush_texture = create_sdl_texture_from_fid(renderer, g_brush_fid);
        }

        if (g_brush_fid != -1) {
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                g_brush_fid = -1;
                if (g_brush_texture) {
                    SDL_DestroyTexture(g_brush_texture);
                    g_brush_texture = nullptr;
                }
            }
        }

        if (g_brush_fid != -1 && g_brush_texture) {
            ImGuiIO& io = ImGui::GetIO();

            int mouse_x = static_cast<int>(io.MousePos.x);
            int mouse_y = static_cast<int>(io.MousePos.y);

            int current_tile = fallout::tileFromScreenXY(mouse_x, mouse_y, fallout::gElevation);

            if (current_tile != -1) {
                int tile_screen_x = 0;
                int tile_screen_y = 0;
                fallout::tileToScreenXY(current_tile, &tile_screen_x, &tile_screen_y);

                int w = 0, h = 0;
                SDL_QueryTexture(g_brush_texture, nullptr, nullptr, &w, &h);

                int screenCenterX = tile_screen_x + 16;
                int screenCenterY = tile_screen_y + 12;

                int offsetX = screenCenterX - (w / 2);
                int offsetY = screenCenterY - h;

                ImVec2 p_min = ImVec2(static_cast<float>(offsetX), static_cast<float>(offsetY));
                ImVec2 p_max = ImVec2(p_min.x + w, p_min.y + h);

                ImU32 alpha_color = IM_COL32(255, 255, 255, 128);

                ImGui::GetForegroundDrawList()->AddImage((ImTextureID)g_brush_texture, p_min, p_max, ImVec2(0, 0),
                                                         ImVec2(1, 1), alpha_color);
            }
        }

        ImGui::End();

        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    }

    bool want_capture_mouse() {
        return g_initialized && g_show_overlay && ImGui::GetIO().WantCaptureMouse;
    }

    bool want_capture_keyboard() {
        return g_initialized && g_show_overlay && ImGui::GetIO().WantCaptureKeyboard;
    }
}
