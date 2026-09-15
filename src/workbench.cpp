#include <SDL2/SDL.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {

constexpr int kLogicalWidth = 1280;
constexpr int kLogicalHeight = 800;

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
    Connector
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

void set_color(SDL_Renderer* renderer,
               std::uint8_t red,
               std::uint8_t green,
               std::uint8_t blue,
               std::uint8_t alpha = 255) {
    SDL_SetRenderDrawColor(renderer, red, green, blue, alpha);
}

void fill_rect(SDL_Renderer* renderer, const RectF& rect) {
    const SDL_FRect value{rect.x, rect.y, rect.w, rect.h};
    SDL_RenderFillRectF(renderer, &value);
}

void draw_rect(SDL_Renderer* renderer, const RectF& rect) {
    const SDL_FRect value{rect.x, rect.y, rect.w, rect.h};
    SDL_RenderDrawRectF(renderer, &value);
}

bool contains(const RectF& rect, float x, float y) {
    return x >= rect.x && x <= rect.x + rect.w &&
           y >= rect.y && y <= rect.y + rect.h;
}

RectF part_rect(PartKind kind, float x, float y) {
    switch (kind) {
        case PartKind::Resistor:
            return {x - 34.0F, y - 13.0F, 68.0F, 26.0F};
        case PartKind::Capacitor:
            return {x - 22.0F, y - 24.0F, 44.0F, 48.0F};
        case PartKind::Led:
            return {x - 20.0F, y - 20.0F, 40.0F, 40.0F};
        case PartKind::Chip:
            return {x - 46.0F, y - 30.0F, 92.0F, 60.0F};
        case PartKind::Power:
            return {x - 36.0F, y - 26.0F, 72.0F, 52.0F};
        case PartKind::Connector:
            return {x - 24.0F, y - 42.0F, 48.0F, 84.0F};
    }
    return {x - 25.0F, y - 20.0F, 50.0F, 40.0F};
}

void draw_part(SDL_Renderer* renderer,
               const PlacedPart& part,
               bool selected) {
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
    set_color(renderer,
              selected ? 255 : 225,
              selected ? 225 : 232,
              selected ? 70 : 235);
    draw_rect(renderer, part.rect);

    const float center_y = part.rect.y + part.rect.h * 0.5F;
    set_color(renderer, 224, 164, 50);
    fill_rect(renderer, {part.rect.x - 7.0F, center_y - 3.0F, 7.0F, 6.0F});
    fill_rect(renderer,
              {part.rect.x + part.rect.w, center_y - 3.0F, 7.0F, 6.0F});
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
    return {palette_width + margin,
            88.0F,
            std::max(280.0F, width - palette_width - margin - margin),
            std::max(220.0F, height - 88.0F - margin)};
}

RectF validate_button_for(float width) {
    return {width - 156.0F, 12.0F, 132.0F, 30.0F};
}

RectF clear_button_for(float width) {
    return {width - 300.0F, 12.0F, 120.0F, 30.0F};
}

bool is_load(PartKind kind) {
    return kind == PartKind::Resistor || kind == PartKind::Led ||
           kind == PartKind::Chip || kind == PartKind::Connector;
}

void run_validation(WorkbenchState& state) {
    state.validation_ran = true;
    state.validation_passed = false;

    for (const Wire& wire : state.wires) {
        if (wire.from >= state.parts.size() || wire.to >= state.parts.size()) {
            continue;
        }

        const PartKind first = state.parts[wire.from].kind;
        const PartKind second = state.parts[wire.to].kind;
        if ((first == PartKind::Power && is_load(second)) ||
            (second == PartKind::Power && is_load(first))) {
            state.validation_passed = true;
            return;
        }
    }
}

void update_window_title(SDL_Window* window, const WorkbenchState& state) {
    std::string title = "FormFactor iOS prototype | Pick: ";
    title += kPartNames[static_cast<std::size_t>(state.selected_kind)];
    title += " | Tap empty board to place | Tap parts to wire";

    if (state.has_wire_start) {
        title += " | CONNECT: tap second part";
    } else if (state.validation_ran) {
        title += state.validation_passed
                     ? " | TEST: PASS - powered load"
                     : " | TEST: NEED POWER + LOAD + WIRE";
    }

    SDL_SetWindowTitle(window, title.c_str());
}

void draw_validate_icon(SDL_Renderer* renderer, const RectF& button) {
    set_color(renderer, 235, 240, 242);
    SDL_RenderDrawLineF(renderer,
                        button.x + 40.0F,
                        button.y + 16.0F,
                        button.x + 55.0F,
                        button.y + 24.0F);
    SDL_RenderDrawLineF(renderer,
                        button.x + 55.0F,
                        button.y + 24.0F,
                        button.x + 88.0F,
                        button.y + 7.0F);
}

void draw_clear_icon(SDL_Renderer* renderer, const RectF& button) {
    set_color(renderer, 235, 240, 242);
    SDL_RenderDrawLineF(renderer,
                        button.x + 45.0F,
                        button.y + 8.0F,
                        button.x + 75.0F,
                        button.y + 22.0F);
    SDL_RenderDrawLineF(renderer,
                        button.x + 75.0F,
                        button.y + 8.0F,
                        button.x + 45.0F,
                        button.y + 22.0F);
}

void draw_workbench(SDL_Renderer* renderer,
                    int width,
                    int height,
                    const WorkbenchState& state) {
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);

    set_color(renderer, 10, 13, 15);
    SDL_RenderClear(renderer);

    set_color(renderer, 18, 24, 27);
    fill_rect(renderer, {0.0F, 0.0F, w, 56.0F});
    set_color(renderer, 57, 255, 20);
    fill_rect(renderer, {0.0F, 54.0F, w, 2.0F});

    const RectF clear_button = clear_button_for(w);
    set_color(renderer, 116, 37, 37);
    fill_rect(renderer, clear_button);
    set_color(renderer, 225, 232, 235);
    draw_rect(renderer, clear_button);
    draw_clear_icon(renderer, clear_button);

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
    draw_validate_icon(renderer, validate_button);

    const float palette_width = palette_width_for(w);
    set_color(renderer, 15, 20, 22);
    fill_rect(renderer, {0.0F, 56.0F, palette_width, h - 56.0F});

    const std::array<float, 6> palette_y{92.0F, 146.0F, 200.0F,
                                         254.0F, 308.0F, 362.0F};
    for (std::size_t index = 0; index < palette_y.size(); ++index) {
        const RectF slot{24.0F,
                         palette_y[index],
                         palette_width - 48.0F,
                         38.0F};
        const bool selected =
            index == static_cast<std::size_t>(state.selected_kind);
        set_color(renderer,
                  selected ? 37 : 28,
                  selected ? 73 : 36,
                  selected ? 45 : 39);
        fill_rect(renderer, slot);
        set_color(renderer, 57, 255, 20);
        draw_rect(renderer, slot);

        const PartKind kind = static_cast<PartKind>(index);
        draw_part(renderer,
                  {kind, part_rect(kind, 45.0F, palette_y[index] + 19.0F)},
                  false);
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
        const RectF& first = state.parts[wire.from].rect;
        const RectF& second = state.parts[wire.to].rect;
        SDL_RenderDrawLineF(renderer,
                            first.x + first.w * 0.5F,
                            first.y + first.h * 0.5F,
                            second.x + second.w * 0.5F,
                            second.y + second.h * 0.5F);
    }

    for (std::size_t index = 0; index < state.parts.size(); ++index) {
        draw_part(renderer,
                  state.parts[index],
                  state.has_wire_start && index == state.wire_start);
    }

    set_color(renderer, 12, 17, 19);
    fill_rect(renderer, {board.x, h - 46.0F, board.w, 28.0F});
    if (state.validation_ran && state.validation_passed) {
        set_color(renderer, 57, 255, 20);
        fill_rect(renderer, {board.x, h - 46.0F, board.w, 3.0F});
    } else if (state.validation_ran) {
        set_color(renderer, 210, 65, 65);
        fill_rect(renderer,
                  {board.x, h - 46.0F, board.w * 0.35F, 3.0F});
    } else {
        set_color(renderer, 57, 255, 20);
        fill_rect(renderer,
                  {board.x, h - 46.0F, board.w * 0.12F, 3.0F});
    }

    SDL_RenderPresent(renderer);
}

bool pick_palette(WorkbenchState& state,
                  float x,
                  float y,
                  float width) {
    const float palette_width = palette_width_for(width);
    const std::array<float, 6> palette_y{92.0F, 146.0F, 200.0F,
                                         254.0F, 308.0F, 362.0F};

    for (std::size_t index = 0; index < palette_y.size(); ++index) {
        if (contains({24.0F,
                      palette_y[index],
                      palette_width - 48.0F,
                      38.0F},
                     x,
                     y)) {
            state.selected_kind = static_cast<PartKind>(index);
            state.validation_ran = false;
            return true;
        }
    }
    return false;
}

bool find_part(const WorkbenchState& state,
               float x,
               float y,
               std::size_t& index) {
    for (std::size_t count = state.parts.size(); count > 0; --count) {
        const std::size_t candidate = count - 1;
        if (contains(state.parts[candidate].rect, x, y)) {
            index = candidate;
            return true;
        }
    }
    return false;
}

void place_part(WorkbenchState& state,
                float x,
                float y,
                const RectF& board) {
    RectF rect = part_rect(state.selected_kind, x, y);
    rect.x = std::clamp(rect.x,
                        board.x + 8.0F,
                        board.x + board.w - rect.w - 8.0F);
    rect.y = std::clamp(rect.y,
                        board.y + 8.0F,
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
        bool duplicate = false;
        for (const Wire& wire : state.wires) {
            if ((wire.from == state.wire_start && wire.to == index) ||
                (wire.from == index && wire.to == state.wire_start)) {
                duplicate = true;
                break;
            }
        }
        if (!duplicate) {
            state.wires.push_back({state.wire_start, index});
            state.validation_ran = false;
        }
    }
    state.has_wire_start = false;
}

void handle_tap(WorkbenchState& state,
                float x,
                float y,
                float width,
                float height) {
    if (contains(clear_button_for(width), x, y)) {
        state = WorkbenchState{};
        return;
    }

    if (contains(validate_button_for(width), x, y)) {
        run_validation(state);
        return;
    }

    if (pick_palette(state, x, y, width)) {
        return;
    }

    const RectF board = board_rect_for(width, height);
    if (!contains(board, x, y)) {
        return;
    }

    std::size_t part_index = 0;
    if (find_part(state, x, y, part_index)) {
        connect_part(state, part_index);
        return;
    }

    place_part(state, x, y, board);
}

}  // namespace

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    SDL_SetHint("SDL_IOS_ORIENTATIONS", "LandscapeLeft LandscapeRight");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "FormFactor iOS prototype",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        kLogicalWidth,
        kLogicalHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    if (window == nullptr) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (renderer == nullptr) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_RenderSetLogicalSize(renderer, kLogicalWidth, kLogicalHeight);

    WorkbenchState state;
    update_window_title(window, state);

    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_INFORMATION,
        "FormFactor iOS Prototype",
        "Tap a part on the left to choose it.\n"
        "Tap empty board space to place it.\n"
        "Tap one placed part, then another, to connect them.\n"
        "Tap the green check button to test the circuit.\n"
        "Tap the red X button to clear the board.",
        window);

    bool running = true;
    while (running) {
        SDL_Event event{};
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
                continue;
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                } else if (event.key.keysym.sym == SDLK_c) {
                    state = WorkbenchState{};
                } else if (event.key.keysym.sym == SDLK_v) {
                    run_validation(state);
                }
                update_window_title(window, state);
                continue;
            }

            if (event.type == SDL_FINGERDOWN) {
                const float x = event.tfinger.x *
                                static_cast<float>(kLogicalWidth);
                const float y = event.tfinger.y *
                                static_cast<float>(kLogicalHeight);
                handle_tap(state,
                           x,
                           y,
                           static_cast<float>(kLogicalWidth),
                           static_cast<float>(kLogicalHeight));
                update_window_title(window, state);
                continue;
            }

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                float x = 0.0F;
                float y = 0.0F;
                SDL_RenderWindowToLogical(renderer,
                                          event.button.x,
                                          event.button.y,
                                          &x,
                                          &y);

                if (event.button.button == SDL_BUTTON_LEFT) {
                    handle_tap(state,
                               x,
                               y,
                               static_cast<float>(kLogicalWidth),
                               static_cast<float>(kLogicalHeight));
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    std::size_t part_index = 0;
                    if (find_part(state, x, y, part_index)) {
                        connect_part(state, part_index);
                    }
                }
                update_window_title(window, state);
            }
        }

        draw_workbench(renderer, kLogicalWidth, kLogicalHeight, state);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
