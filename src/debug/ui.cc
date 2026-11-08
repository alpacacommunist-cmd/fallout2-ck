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

SDL_Texture* create_sdl_texture_from_fid(fallout::FrmId frm_id) {
    fallout::CacheEntry* entry = nullptr;
    fallout::Art* art = fallout::artLock(frm_id, &entry);

    if (!entry) return nullptr;
    int art_height = fallout::artGetHeight(art, 0, static_cast<fallout::Rotation>(0));
    int art_width = fallout::artGetWidth(art, 0, static_cast<fallout::Rotation>(0));

    // if (frm_id.objectType() == fallout::ObjectType::OBJ_TYPE_SCENERY) {
    // }

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

    SDL_Texture* texture = SDL_CreateTextureFromSurface(fallout::gSdlRenderer, surface);

    SDL_FreeSurface(surface);
    SDL_FreePalette(pool_palette);

    return texture;
}

namespace ck::debug::ui {
    struct CachedTexture {
        SDL_Texture* texture = nullptr;
        int width = 0;
        int height = 0;
        int offset_x = 0;
        int offset_y = 0;
    };
    static std::unordered_map<int, CachedTexture> g_asset_texture_cache;

    static bool g_initialized = false;

    static CachedTexture* g_cursor = nullptr;
    static CachedTexture* g_brush = nullptr;
    static int g_brush_fid = -1;

    static CachedTexture* get_or_create_texture(fallout::FrmId frm_id) {
        int fid = frm_id.fid();

        auto it = g_asset_texture_cache.find(fid);
        if (it != g_asset_texture_cache.end()) return &it->second;

        CachedTexture entry;
        entry.texture = create_sdl_texture_from_fid(frm_id);
        if (entry.texture) {
            SDL_QueryTexture(entry.texture, nullptr, nullptr, &entry.width, &entry.height);
            g_asset_texture_cache[fid] = entry;

            return &g_asset_texture_cache[fid];
        }

        return nullptr;
    }

    static void render_brush() {
        if (g_brush_fid == -1 || !g_brush) return;

        ImGuiIO& io = ImGui::GetIO();

        int mouse_x = static_cast<int>(io.MousePos.x);
        int mouse_y = static_cast<int>(io.MousePos.y);

        int current_tile = fallout::tileFromScreenXY(mouse_x, mouse_y, fallout::gElevation);

        if (current_tile != -1) {
            int tile_screen_x = 0;
            int tile_screen_y = 0;
            fallout::tileToScreenXY(current_tile, &tile_screen_x, &tile_screen_y);

            int screen_center_x = tile_screen_x + 16;
            int screen_center_y = tile_screen_y + 12;
            int offset_x = screen_center_x - (g_brush->width / 2);
            int offset_y = screen_center_y - g_brush->height;

            SDL_Rect dst_rect = {offset_x, offset_y, g_brush->width, g_brush->height};

            SDL_SetTextureBlendMode(g_brush->texture, SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(g_brush->texture, 128);

            SDL_RenderCopy(fallout::gSdlRenderer, g_brush->texture, nullptr, &dst_rect);

            // ImVec2 p_min = ImVec2(static_cast<float>(offset_x), static_cast<float>(offset_y));
            // ImVec2 p_max = ImVec2(p_min.x + g_brush->width, p_min.y + g_brush->height);
            //
            // ImU32 alpha_color = IM_COL32(255, 255, 255, 128);
            //
            // ImGui::GetForegroundDrawList()->AddImage((ImTextureID)g_brush->texture, p_min, p_max, ImVec2(0, 0),
            //                                          ImVec2(1, 1), alpha_color);
        }
    }

    static void render_cursor(ImGuiIO& io) {
        if (!g_cursor || !g_cursor->texture) return;

        int mouse_x = static_cast<int>(io.MousePos.x);
        int mouse_y = static_cast<int>(io.MousePos.y);

        SDL_Rect dst_rect = { mouse_x, mouse_y, g_cursor->width, g_cursor->height };

        SDL_SetTextureBlendMode(g_cursor->texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(g_cursor->texture, 255);

        SDL_RenderCopy(fallout::gSdlRenderer, g_cursor->texture, nullptr, &dst_rect);

        // int w = 0, h = 0;
        // SDL_QueryTexture(g_cursor->texture, nullptr, nullptr, &w, &h);
        //
        // ImVec2 mouse_pos = io.MousePos;
        //
        // ImVec2 p_min = mouse_pos;
        // ImVec2 p_max = ImVec2(mouse_pos.x + w, mouse_pos.y + h);
        //
        // ImGui::GetForegroundDrawList()->AddImage((ImTextureID)g_cursor->texture, p_min, p_max, ImVec2(0, 0),
        //         ImVec2(1, 1), IM_COL32(255, 255, 255, 255));
    }

    void initialize() {
        if (g_initialized) return;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();

        // keyboard
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        fallout::FrmId cursor_frm = fallout::InterfaceFrameId::MouseCursorStandardArrow;
        g_cursor = get_or_create_texture(cursor_frm);

        ImGui_ImplSDL2_InitForSDLRenderer(fallout::gSdlWindow, fallout::gSdlRenderer);
        ImGui_ImplSDLRenderer2_Init(fallout::gSdlRenderer);

        g_initialized = true;
    }

    void shutdown() {
        if (!g_initialized) return;

        if (g_cursor) {
            SDL_DestroyTexture(g_cursor->texture);
            g_cursor = nullptr;
        }

        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        for (auto& [fid, cached] : g_asset_texture_cache) {
            if (cached.texture) SDL_DestroyTexture(cached.texture);
        }
        g_asset_texture_cache.clear();

        g_brush_fid = -1;
        g_brush = nullptr;

        g_initialized = false;
    }

    void process_event(void *event) {
        if (!g_initialized || !(ck::debug::enabled())) return;

        ImGui_ImplSDL2_ProcessEvent(static_cast<SDL_Event*>(event));
    }

    void render() {
        if (!g_initialized || !ck::debug::enabled()) return;

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();

        SDL_ShowCursor(SDL_DISABLE);

        ImGui::NewFrame();
        ImGuiIO& io = ImGui::GetIO();

        if (io.WantCaptureMouse) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        }

        ImGui::Begin("Fallout 2 CK - Asset Manager");
        ImGui::Text("Hello from LuaJIT runtime workspace!");

        if (ImGui::Button("Spawn Scenery (Tree)")) {
            logger.info.banner("BRUSH ACTIVATED!");

            if (g_brush) {
                SDL_DestroyTexture(g_brush->texture);
                g_brush_fid = -1;
            }

            g_brush_fid = 33555377;
            g_brush = get_or_create_texture(fallout::FrmId(g_brush_fid));
        }

        if (g_brush_fid != -1) {
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                g_brush_fid = -1;
                g_brush = nullptr;
            }
        }

        render_brush();

        ImGui::End();

        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), fallout::gSdlRenderer);

        if (io.WantCaptureMouse && g_cursor) {
            render_cursor(io);
        }
    }

    bool want_capture_mouse() {
        return g_initialized && ck::debug::enabled() && ImGui::GetIO().WantCaptureMouse;
    }

    bool want_capture_keyboard() {
        return g_initialized && ck::debug::enabled() && ImGui::GetIO().WantCaptureKeyboard;
    }
}
