#ifndef CK_PUBLIC_UI_H
#define CK_PUBLIC_UI_H

namespace ck::debug::ui {
    void process_event(SDL_Event *event);
    void render(SDL_Renderer *renderer);

    bool want_capture_mouse();
    bool want_capture_keyboard();
} // namespace ck::ui

#endif
