#include <SDL2/SDL.h>

namespace {

SDL_HitTestResult SDLCALL formfactor_window_hit_test(SDL_Window* window,
                                                      const SDL_Point* point,
                                                      void*) {
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(window, &width, &height);
    (void)height;

    // Keep gameplay clicks on the board and toolbar working normally. The
    // left side of FormFactor's 72 px app header behaves like a title bar,
    // so the player can grab the app itself and move the whole window.
    if (point != nullptr && point->y >= 0 && point->y < 72 &&
        point->x >= 0 && point->x < width && point->x < 430) {
        return SDL_HITTEST_DRAGGABLE;
    }
    return SDL_HITTEST_NORMAL;
}

SDL_Window* formfactor_create_window(const char* title, int x, int y, int w,
                                     int h, Uint32 flags) {
    SDL_Window* window = SDL_CreateWindow(title, x, y, w, h, flags);
    if (window != nullptr) {
        SDL_SetWindowHitTest(window, formfactor_window_hit_test, nullptr);
        SDL_SetWindowMinimumSize(window, 960, 620);
    }
    return window;
}

}  // namespace

// Intercept the prototype's window creation without making it borderless.
// SDL therefore keeps the OS-native minimize, maximize/restore, and close
// buttons while also making the in-app FormFactor header draggable.
#define SDL_CreateWindow formfactor_create_window

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include "workbench_plus.cpp"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
