#include <SDL2/SDL.h>

#include <algorithm>
#include <array>
#include <cstdint>

namespace {

struct RectF {
    float x;
    float y;
    float w;
    float h;
};

void set_color(SDL_Renderer* renderer, std::uint8_t r, std::uint8_t g,
               std::uint8_t b, std::uint8_t a = 255) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void fill_rect(SDL_Renderer* renderer, const RectF& rect) {
    const SDL_FRect sdl_rect{rect.x, rect.y, rect.w, rect.h};
    SDL_RenderFillRectF(renderer, &sdl_rect);
}

void draw_rect(SDL_Renderer* renderer, const RectF& rect) {
    const SDL_FRect sdl_rect{rect.x, rect.y, rect.w, rect.h};
    SDL_RenderDrawRectF(renderer, &sdl_rect);
}

void draw_workbench(SDL_Renderer* renderer, int width, int height) {
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);

    // Background and top bar.
    set_color(renderer, 10, 13, 15);
    SDL_RenderClear(renderer);

    set_color(renderer, 18, 24, 27);
    fill_rect(renderer, {0.0F, 0.0F, w, 56.0F});

    set_color(renderer, 57, 255, 20);
    fill_rect(renderer, {0.0F, 54.0F, w, 2.0F});

    // Left component palette.
    const float palette_width = std::min(230.0F, w * 0.24F);
    set_color(renderer, 15, 20, 22);
    fill_rect(renderer, {0.0F, 56.0F, palette_width, h - 56.0F});

    const std::array<float, 6> palette_y{92.0F, 146.0F, 200.0F, 254.0F,
                                        308.0F, 362.0F};
    for (const float y : palette_y) {
        set_color(renderer, 28, 36, 39);
        fill_rect(renderer, {24.0F, y, palette_width - 48.0F, 38.0F});
        set_color(renderer, 57, 255, 20);
        draw_rect(renderer, {24.0F, y, palette_width - 48.0F, 38.0F});
    }

    // Main PCB canvas.
    const float margin = 32.0F;
    const float board_x = palette_width + margin;
    const float board_y = 88.0F;
    const float board_w = std::max(280.0F, w - board_x - margin);
    const float board_h = std::max(220.0F, h - board_y - margin);

    set_color(renderer, 18, 54, 43);
    fill_rect(renderer, {board_x, board_y, board_w, board_h});
    set_color(renderer, 57, 255, 20);
    draw_rect(renderer, {board_x, board_y, board_w, board_h});

    // Board grid.
    set_color(renderer, 28, 78, 63);
    for (float x = board_x + 24.0F; x < board_x + board_w; x += 24.0F) {
        SDL_RenderDrawLineF(renderer, x, board_y, x, board_y + board_h);
    }
    for (float y = board_y + 24.0F; y < board_y + board_h; y += 24.0F) {
        SDL_RenderDrawLineF(renderer, board_x, y, board_x + board_w, y);
    }

    // Visual-only example components: controller, power stage, connector.
    const RectF controller{board_x + board_w * 0.42F, board_y + board_h * 0.32F,
                           120.0F, 82.0F};
    set_color(renderer, 22, 25, 28);
    fill_rect(renderer, controller);
    set_color(renderer, 210, 220, 224);
    draw_rect(renderer, controller);

    // IC pads.
    set_color(renderer, 212, 167, 58);
    for (int i = 0; i < 6; ++i) {
        const float py = controller.y + 7.0F + static_cast<float>(i) * 12.0F;
        fill_rect(renderer, {controller.x - 8.0F, py, 8.0F, 5.0F});
        fill_rect(renderer, {controller.x + controller.w, py, 8.0F, 5.0F});
    }

    // Power device.
    const RectF power{board_x + board_w * 0.18F, board_y + board_h * 0.58F,
                      72.0F, 54.0F};
    set_color(renderer, 31, 34, 37);
    fill_rect(renderer, power);
    set_color(renderer, 210, 220, 224);
    draw_rect(renderer, power);

    // Connector.
    const RectF connector{board_x + board_w - 100.0F, board_y + board_h * 0.44F,
                          56.0F, 112.0F};
    set_color(renderer, 22, 25, 28);
    fill_rect(renderer, connector);
    set_color(renderer, 210, 220, 224);
    draw_rect(renderer, connector);

    // Example routed copper between the visual-only blocks.
    set_color(renderer, 232, 117, 17);
    SDL_RenderDrawLineF(renderer,
                        power.x + power.w,
                        power.y + power.h * 0.5F,
                        controller.x,
                        controller.y + controller.h * 0.65F);
    SDL_RenderDrawLineF(renderer,
                        controller.x + controller.w,
                        controller.y + controller.h * 0.5F,
                        connector.x,
                        connector.y + connector.h * 0.5F);

    // Status strip.
    set_color(renderer, 12, 17, 19);
    fill_rect(renderer, {board_x, h - 46.0F, board_w, 28.0F});
    set_color(renderer, 57, 255, 20);
    fill_rect(renderer, {board_x, h - 46.0F, board_w * 0.35F, 3.0F});

    SDL_RenderPresent(renderer);
}

}  // namespace

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "FormFactor / FreeLab - Visual Workbench (prototype)",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        800,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (window == nullptr) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }

    if (renderer == nullptr) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    while (running) {
        SDL_Event event{};
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }

        int width = 0;
        int height = 0;
        SDL_GetRendererOutputSize(renderer, &width, &height);
        draw_workbench(renderer, width, height);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
