#include <SDL2/SDL.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {

struct RectF {
    float x;
    float y;
    float w;
    float h;
};

enum class PartKind : std::size_t {
    Resistor = 0,
    Capacitor,
    Led,
    Chip,
    Power,
    Connector,
};

struct PlacedPart {
    PartKind kind;
    RectF rect;
};

struct Wire {
    std::size_t from;
    std::size_t to;
};

constexpr std::array<const char*, 6> kPartNames{
    "Resistor", "Capacitor", "LED", "Chip", "Power", "Connector"};

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

bool contains(const RectF& rect, float x, float y) {
    return x >= rect.x && x <= rect.x + rect.w && y >= rect.y &&
           y <= rect.y + rect.h;
}

RectF part_rect(PartKind kind, float center_x, float center_y) {
    switch (kind) {
        case PartKind::Resistor:
            return {center_x - 34.0F, center_y - 13.0F, 68.0F, 26.0F};
        case PartKind::Capacitor:
            return {center_x - 22.0F, center_y - 24.0F, 44.0F, 48.0F};
        case PartKind::Led:
            return {center_x - 20.0F, center_y - 20.0F, 40.0F, 40.0F};
        case PartKind::Chip:
            return {center_x - 46.0F, center_y - 30.0F, 92.0F, 60.0F};
        case PartKind::Power:
            return {center_x - 36.0F, center_y - 26.0F, 72.0F, 52.0F};
        case PartKind::Connector:
            return {center_x - 24.0F, center_y - 42.0F, 48.0F, 84.0F};
    }
    return {center_x - 25.0F, center_y - 20.0F, 50.0F, 40.0F};
}

void draw_part(SDL_Renderer* renderer, const PlacedPart& part, bool selected) {
    switch (part.kind) {
        case PartKind::Resistor:
            set_color(renderer, 190, 132, 69);
            break;
        case PartKind::Capacitor:
            set_color(renderer, 56, 122, 209);
            break;
        case PartKind::Led:
            set_color(renderer, 57, 255, 20);
            break;
        case PartKind::Chip:
            set_color(renderer, 35, 39, 44);
            break;
        case PartKind::Power:
            set_color(renderer, 196, 58, 58);
            break;
        case PartKind::Connector:
            set_color(renderer, 201, 210, 214);
            break;
    }
    fill_rect(renderer, part.rect);

    if (selected) {
        set_color(renderer, 255, 225, 70);
    } else {
        set_color(renderer, 225, 232, 235);
    }
    draw_rect(renderer, part.rect);

    const float cy = part.rect.y + part.rect.h * 0.5F;
    set_color(renderer, 224, 164, 50);
    fill_rect(renderer, {part.rect.x - 7.0F, cy - 3.0F, 7.0F, 6.0F});
    fill_rect(renderer,
              {part.rect.x + part.rect.w, cy - 3.0F, 7.0F, 6.0F});
}

struct WorkbenchState {
    PartKind selected_kind{PartKind::Resistor};
    std::vector<PlacedPart> parts;
    std::vector<Wire> wires;
    bool has_wire_start{false};
    std::size_t wire_start{0};
    bool validation_ran{false};
    bool validation_passed{false};
};

float palette_width_for(float width) {
    return std::min(230.0F, width * 0.24F);
}

RectF board_rect_for(float width, float height) {
    const float palette_width = palette_width_for(width);
    const float margin = 32.0F;
    const float board_x = palette_width + margin;
    const float board_y = 88.0F;
    const float board_w = std::max(280.0F, width - board_x - margin);
    const float board_h = std::max(220.0F, height - board_y - margin);
    return {board_x, board_y, board_w, board_h};
}

RectF validate_button_for(float width) {
    return {width - 156.0F, 12.0F, 132.0F, 30.0F};
}

void run_validation(WorkbenchState& state) {
    state.validation_ran = true;
    state.validation_passed = state.parts.size() >= 2 && !state.wires.empty();
}

void update_window_title(SDL_Window* window, const WorkbenchState& state) {
    const auto selected_index = static_cast<std::size_t>(state.selected_kind);
    std::string title = "FormFactor prototype | Pick: ";
    title += kPartNames[selected_index];
    title += " | Left click place | Right click 2 parts connect | V test | C clear";

    if (state.has_wire_start) {
        title += " | CONNECT: choose second part";
    } else if (state.validation_ran) {
        title += state.validation_passed ? " | TEST: PASS" : " | TEST: NEED 2 PARTS + 1 WIRE";
    }

    SDL_SetWindowTitle(window, title.c_str());
}

void draw_workbench(SDL_Renderer* renderer, int width, int height,
                    const WorkbenchState& state) {
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);

    set_color(renderer, 10, 13, 15);
    SDL_RenderClear(renderer);

    set_color(renderer, 18, 24, 27);
    fill_rect(renderer, {0.0F, 0.0F, w, 56.0F});
    set_color(renderer, 57, 255, 20);
    fill_rect(renderer, {0.0F, 54.0F, w, 2.0F});

    const RectF validate_button = validate_button_for(w);
    if (state.validation_ran && state.validation_passed) {
        set_color(renderer, 57, 255, 20);
    } else if (state.validation_ran) {
        set_color(renderer, 210, 65, 65);
    } else {
        set_color(renderer, 38, 86, 48);
    }
    fill_rect(renderer, validate_button);
    set_color(renderer, 225, 232, 235);
    draw_rect(renderer, validate_button);

    const float palette_width = palette_width_for(w);
    set_color(renderer, 15, 20, 22);
    fill_rect(renderer, {0.0F, 56.0F, palette_width, h - 56.0F});

    const std::array<float, 6> palette_y{92.0F, 146.0F, 200.0F, 254.0F,
                                        308.0F, 362.0F};
    for (std::size_t i = 0; i < palette_y.size(); ++i) {
        const RectF slot{24.0F, palette_y[i], palette_width - 48.0F, 38.0F};
        if (i == static_cast<std::size_t>(state.selected_kind)) {
            set_color(renderer, 37, 73, 45);
        } else {
            set_color(renderer, 28, 36, 39);
        }
        fill_rect(renderer, slot);
        set_color(renderer, 57, 255, 20);
        draw_rect(renderer, slot);

        const PartKind kind = static_cast<PartKind>(i);
        const PlacedPart preview{kind, part_rect(kind, 45.0F, palette_y[i] + 19.0F)};
        draw_part(renderer, preview, false);
    }

    const RectF board = board_rect_for(w, h);
    set_color(renderer, 18, 54, 43);
    fill_rect(renderer, board);
    set_color(renderer, 57, 255, 20);
    draw_rect(renderer, board);

    set_color(renderer, 28, 78, 63);
    for (float x = board.x + 24.0F; x < board.x + board.w; x += 24.0F) {
        SDL_RenderDrawLineF(renderer, x, board.y, x, board.y + board.h);
    }
    for (float y = board.y + 24.0F; y < board.y + board.h; y += 24.0F) {
        SDL_RenderDrawLineF(renderer, board.x, y, board.x + board.w, y);
    }

    set_color(renderer, 232, 117, 17);
    for (const Wire& wire : state.wires) {
        if (wire.from >= state.parts.size() || wire.to >= state.parts.size()) {
            continue;
        }
        const RectF& a = state.parts[wire.from].rect;
        const RectF& b = state.parts[wire.to].rect;
        SDL_RenderDrawLineF(renderer, a.x + a.w * 0.5F, a.y + a.h * 0.5F,
                            b.x + b.w * 0.5F, b.y + b.h * 0.5F);
    }

    for (std::size_t i = 0; i < state.parts.size(); ++i) {
        draw_part(renderer, state.parts[i], state.has_wire_start && i == state.wire_start);
    }

    set_color(renderer, 12, 17, 19);
    fill_rect(renderer, {board.x, h - 46.0F, board.w, 28.0F});
    if (state.validation_ran && state.validation_passed) {
        set_color(renderer, 57, 255, 20);
        fill_rect(renderer, {board.x, h - 46.0F, board.w, 3.0F});
    } else if (state.validation_ran) {
        set_color(renderer, 210, 65, 65);
        fill_rect(renderer, {board.x, h - 46.0F, board.w * 0.35F, 3.0F});
    } else {
        set_color(renderer, 57, 255, 20);
        fill_rect(renderer, {board.x, h - 46.0F, board.w * 0.12F, 3.0F});
    }

    SDL_RenderPresent(renderer);
}

bool pick_palette(WorkbenchState& state, float x, float y, float width) {
    const float palette_width = palette_width_for(width);
    const std::array<float, 6> palette_y{92.0F, 146.0F, 200.0F, 254.0F,
                                        308.0F, 362.0F};
    for (std::size_t i = 0; i < palette_y.size(); ++i) {
        const RectF slot{24.0F, palette_y[i], palette_width - 48.0F, 38.0F};
        if (contains(slot, x, y)) {
            state.selected_kind = static_cast<PartKind>(i);
            state.validation_ran = false;
            return true;
        }
    }
    return false;
}

bool find_part(const WorkbenchState& state, float x, float y, std::size_t& index) {
    for (std::size_t i = state.parts.size(); i > 0; --i) {
        const std::size_t candidate = i - 1;
        if (contains(state.parts[candidate].rect, x, y)) {
            index = candidate;
            return true;
        }
    }
    return false;
}

void place_part(WorkbenchState& state, float x, float y, const RectF& board) {
    RectF rect = part_rect(state.selected_kind, x, y);
    rect.x = std::clamp(rect.x, board.x + 8.0F,
                        board.x + board.w - rect.w - 8.0F);
    rect.y = std::clamp(rect.y, board.y + 8.0F,
                        board.y + board.h - rect.h - 8.0F);
    state.parts.push_back({state.selected_kind, rect});
    state.validation_ran = false;
}

void connect_part(WorkbenchState& state, std::size_t index) {
    if (!state.has_wire_start) {
        state.has_wire_start = true;
        state.wire_start = index;
        return;
    }

    if (state.wire_start != index) {
        state.wires.push_back({state.wire_start, index});
        state.validation_ran = false;
    }
    state.has_wire_start = false;
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
        "FormFactor prototype",
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

    WorkbenchState state;
    update_window_title(window, state);

    bool running = true;
    while (running) {
        SDL_Event event{};
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                } else if (event.key.keysym.sym == SDLK_c) {
                    state = WorkbenchState{};
                } else if (event.key.keysym.sym == SDLK_v) {
                    run_validation(state);
                }
                update_window_title(window, state);
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int width = 0;
                int height = 0;
                SDL_GetRendererOutputSize(renderer, &width, &height);
                const float x = static_cast<float>(event.button.x);
                const float y = static_cast<float>(event.button.y);
                const float w = static_cast<float>(width);
                const float h = static_cast<float>(height);
                const RectF board = board_rect_for(w, h);

                if (event.button.button == SDL_BUTTON_LEFT) {
                    if (contains(validate_button_for(w), x, y)) {
                        run_validation(state);
                    } else if (!pick_palette(state, x, y, w) && contains(board, x, y)) {
                        place_part(state, x, y, board);
                    }
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    std::size_t index = 0;
                    if (find_part(state, x, y, index)) {
                        connect_part(state, index);
                    }
                }
                update_window_title(window, state);
            }
        }

        int width = 0;
        int height = 0;
        SDL_GetRendererOutputSize(renderer, &width, &height);
        draw_workbench(renderer, width, height, state);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
