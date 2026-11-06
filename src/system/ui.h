#pragma once
#include <SDL.h>

namespace ck::ui {
    void initialize(SDL_Window *window, SDL_Renderer *renderer);
    void shutdown();

    void process_event(SDL_Event *event);
    void render(SDL_Renderer *renderer);

    bool want_capture_mouse();
    bool want_capture_keyboard();
} // namespace ck::ui
