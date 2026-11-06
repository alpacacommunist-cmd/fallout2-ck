#include "ui.h"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/backends/imgui_impl_sdl2.h"
#include "vendor/imgui/backends/imgui_impl_sdlrenderer2.h"

#include "log.h"
static logging::Context logger{"ck_ui.cc", logging::Source::Native};

namespace ck::ui {
    static bool g_initialized = false;
    static bool g_show_overlay = false;

    void initialize(SDL_Window *window, SDL_Renderer *renderer) {
        if (g_initialized) return;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer2_Init(renderer);

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

        if (event->type == SDL_KEYDOWN && event->key.keysym.scancode == SDL_SCANCODE_GRAVE && !event->key.repeat) {
            g_show_overlay = !g_show_overlay;
        }

        if (g_show_overlay) { ImGui_ImplSDL2_ProcessEvent(event); }
    }

    void render(SDL_Renderer *renderer) {
        if (!g_initialized || !g_show_overlay) return;

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Fallout 2 CK - Asset Manager", &g_show_overlay);
        ImGui::Text("Hello from LuaJIT runtime workspace!");

        if (ImGui::Button("Spawn Scenery (Tree)")) {
            logger.info.banner("SPAWNING!");
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
} // namespace ck::ui
