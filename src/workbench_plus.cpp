#define main formfactor_legacy_main
#include "workbench.cpp"
#undef main

namespace {

struct InteractionState {
    float mouse_x{0.0F};
    float mouse_y{0.0F};
    bool mouse_known{false};
    bool dragging{false};
    bool drag_moved{false};
    std::size_t drag_index{0};
    float drag_offset_x{0.0F};
    float drag_offset_y{0.0F};
    RectF drag_origin{};
};

float snap_to_half_grid(float value, float origin) {
    constexpr float step = 12.0F;
    return origin + std::round((value - origin) / step) * step;
}

RectF clamp_part_rect(RectF q, const RectF& board) {
    q.x = std::clamp(q.x, board.x + 8.0F, board.x + board.w - q.w - 8.0F);
    q.y = std::clamp(q.y, board.y + 24.0F, board.y + board.h - q.h - 8.0F);
    return q;
}

RectF snapped_part_rect(PartKind kind, float x, float y, const RectF& board) {
    const float sx = snap_to_half_grid(x, board.x);
    const float sy = snap_to_half_grid(y, board.y);
    return clamp_part_rect(part_rect(kind, sx, sy), board);
}

void place_part_snapped(WorkbenchState& s, float x, float y, const RectF& board) {
    const RectF q = snapped_part_rect(s.selected_kind, x, y, board);
    s.parts.push_back({s.selected_kind, q});
    s.board_focus = s.parts.size() - 1;
    s.focus = FocusZone::BoardPart;
    s.validation_ran = false;
    s.status = std::string("PLACED ") + reference_for(s.parts, s.board_focus) +
               ". DRAG TO MOVE. DELETE REMOVES IT.";
}

void remove_focused_part(WorkbenchState& s) {
    if (s.focus != FocusZone::BoardPart || s.parts.empty() || s.board_focus >= s.parts.size()) {
        s.status = "FOCUS A BOARD PART BEFORE DELETE.";
        return;
    }

    const std::size_t removed = s.board_focus;
    const std::string removed_name = reference_for(s.parts, removed);

    s.wires.erase(
        std::remove_if(s.wires.begin(), s.wires.end(), [removed](const Wire& wire) {
            return wire.from == removed || wire.to == removed;
        }),
        s.wires.end());

    for (Wire& wire : s.wires) {
        if (wire.from > removed) --wire.from;
        if (wire.to > removed) --wire.to;
    }

    if (s.has_wire_start) {
        if (s.wire_start == removed) {
            s.has_wire_start = false;
        } else if (s.wire_start > removed) {
            --s.wire_start;
        }
    }

    s.parts.erase(s.parts.begin() + static_cast<std::ptrdiff_t>(removed));
    s.validation_ran = false;
    s.validation_passed = false;

    if (s.parts.empty()) {
        s.board_focus = 0;
        s.focus = FocusZone::Palette;
    } else {
        s.board_focus = std::min(removed, s.parts.size() - 1);
        s.focus = FocusZone::BoardPart;
    }

    s.status = "REMOVED " + removed_name + ". CONNECTED WIRES WERE CLEANED UP.";
}

bool cancel_transient_action(WorkbenchState& s, InteractionState& interaction) {
    if (s.show_help) {
        s.show_help = false;
        s.status = "HELP CLOSED.";
        return true;
    }
    if (interaction.dragging && interaction.drag_index < s.parts.size()) {
        s.parts[interaction.drag_index].rect = interaction.drag_origin;
        interaction.dragging = false;
        interaction.drag_moved = false;
        s.status = "MOVE CANCELLED.";
        return true;
    }
    if (s.has_wire_start) {
        s.has_wire_start = false;
        s.status = "CONNECTION CANCELLED.";
        return true;
    }
    return false;
}

void begin_drag(WorkbenchState& s, InteractionState& interaction,
                std::size_t index, float x, float y) {
    if (index >= s.parts.size()) return;
    s.focus = FocusZone::BoardPart;
    s.board_focus = index;
    interaction.dragging = true;
    interaction.drag_moved = false;
    interaction.drag_index = index;
    interaction.drag_origin = s.parts[index].rect;
    interaction.drag_offset_x = x - s.parts[index].rect.x;
    interaction.drag_offset_y = y - s.parts[index].rect.y;
    s.status = std::string("SELECTED ") + reference_for(s.parts, index) +
               ". DRAG TO MOVE OR DELETE TO REMOVE.";
}

void update_drag(WorkbenchState& s, InteractionState& interaction, const RectF& board) {
    if (!interaction.dragging || interaction.drag_index >= s.parts.size()) return;

    RectF q = s.parts[interaction.drag_index].rect;
    const float raw_x = interaction.mouse_x - interaction.drag_offset_x;
    const float raw_y = interaction.mouse_y - interaction.drag_offset_y;
    q.x = snap_to_half_grid(raw_x, board.x);
    q.y = snap_to_half_grid(raw_y, board.y);
    q = clamp_part_rect(q, board);

    const RectF old = s.parts[interaction.drag_index].rect;
    if (std::fabs(old.x - q.x) > 0.1F || std::fabs(old.y - q.y) > 0.1F) {
        s.parts[interaction.drag_index].rect = q;
        interaction.drag_moved = true;
        s.validation_ran = false;
    }
}

void draw_double_rect(SDL_Renderer* r, const RectF& q) {
    draw_rect(r, q);
    if (q.w > 4.0F && q.h > 4.0F) {
        draw_rect(r, {q.x + 2.0F, q.y + 2.0F, q.w - 4.0F, q.h - 4.0F});
    }
}

void draw_interaction_overlay(SDL_Renderer* r, float w, float h,
                              const WorkbenchState& s,
                              const InteractionState& interaction) {
    if (!interaction.mouse_known || s.show_help) return;

    const RectF board = board_rect_for(w, h);
    const bool on_board = contains(board, interaction.mouse_x, interaction.mouse_y);

    if (s.has_wire_start && s.wire_start < s.parts.size()) {
        const RectF& start = s.parts[s.wire_start].rect;
        const float sx = start.x + start.w * 0.5F;
        const float sy = start.y + start.h * 0.5F;

        if (on_board) {
            set_color(r, 255, 190, 35, 220);
            SDL_RenderDrawLineF(r, sx, sy, interaction.mouse_x, interaction.mouse_y);
            SDL_RenderDrawLineF(r, sx + 1.0F, sy + 1.0F,
                               interaction.mouse_x + 1.0F, interaction.mouse_y + 1.0F);
        }

        for (std::size_t i = 0; i < s.parts.size(); ++i) {
            if (i == s.wire_start) continue;
            if (!prototype_pair_allowed(s.parts[s.wire_start].kind, s.parts[i].kind)) continue;
            const bool hovered = contains(s.parts[i].rect, interaction.mouse_x, interaction.mouse_y);
            set_color(r, hovered ? 255 : 57, hovered ? 225 : 255,
                      hovered ? 70 : 20, 235);
            const RectF halo{s.parts[i].rect.x - 5.0F, s.parts[i].rect.y - 5.0F,
                             s.parts[i].rect.w + 10.0F, s.parts[i].rect.h + 10.0F};
            draw_double_rect(r, halo);
        }
        return;
    }

    if (interaction.dragging) {
        if (interaction.drag_index < s.parts.size()) {
            set_color(r, 255, 225, 70, 235);
            const RectF& p = s.parts[interaction.drag_index].rect;
            draw_double_rect(r, {p.x - 5.0F, p.y - 5.0F, p.w + 10.0F, p.h + 10.0F});
        }
        return;
    }

    if (!on_board) return;

    std::size_t hovered_index = 0;
    if (find_part(s, interaction.mouse_x, interaction.mouse_y, hovered_index)) {
        const RectF& p = s.parts[hovered_index].rect;
        set_color(r, 57, 255, 20, 210);
        draw_rect(r, {p.x - 3.0F, p.y - 3.0F, p.w + 6.0F, p.h + 6.0F});
        return;
    }

    const RectF ghost = snapped_part_rect(s.selected_kind,
                                          interaction.mouse_x,
                                          interaction.mouse_y,
                                          board);
    set_color(r, 57, 255, 20, 46);
    fill_rect(r, ghost);
    set_color(r, 57, 255, 20, 220);
    draw_double_rect(r, ghost);
    draw_schematic_symbol(r, s.selected_kind,
                          {ghost.x + 5.0F, ghost.y + 5.0F,
                           std::max(18.0F, ghost.w - 10.0F),
                           std::max(18.0F, ghost.h - 10.0F)}, true);

    const float label_y = std::max(board.y + 5.0F, ghost.y - 17.0F);
    set_color(r, 225, 239, 232);
    draw_text(r, "CLICK TO PLACE", ghost.x, label_y, 0.75F);
}

void draw_enhanced_help_overlay(SDL_Renderer* r, float w, float h) {
    draw_help_overlay(r, w, h);
    const RectF q{w * 0.13F, h * 0.10F, w * 0.74F, h * 0.78F};
    const float left = q.x + 30.0F;
    const float right = q.x + q.w * 0.52F;
    const float column_w = q.w * 0.42F;

    set_color(r, 3, 6, 7, 252);
    fill_rect(r, {left - 4.0F, q.y + 340.0F, column_w + 8.0F, 190.0F});
    fill_rect(r, {right - 4.0F, q.y + 82.0F, column_w + 8.0F, 260.0F});

    set_color(r, 57, 255, 20);
    draw_text(r, "MOUSE", left, q.y + 350.0F, 1.35F);
    set_color(r, 210, 221, 223);
    draw_wrapped_text(
        r,
        "MOVE OVER EMPTY BOARD SPACE TO PREVIEW A SNAPPED PART. CLICK TO PLACE. CLICK AND DRAG A PLACED PART TO MOVE IT. RIGHT CLICK ONE PART THEN ANOTHER TO CONNECT. VALID TARGETS GLOW AND A LIVE WIRE FOLLOWS THE POINTER.",
        left, q.y + 378.0F, column_w, 1.0F, 5.0F);

    set_color(r, 57, 255, 20);
    draw_text(r, "KEYBOARD", right, q.y + 92.0F, 1.45F);
    set_color(r, 224, 232, 234);
    draw_wrapped_text(
        r,
        "TAB: NEXT GROUP. SHIFT + TAB: PREVIOUS. ARROWS: BROWSE. ENTER OR SPACE: USE FOCUS. DELETE OR BACKSPACE: REMOVE FOCUSED PART. V: TEST. C: CLEAR. H OR F1: HELP. ESC: CANCEL HELP, MOVE, OR WIRE FIRST; PRESS AGAIN WITH NOTHING ACTIVE TO EXIT.",
        right, q.y + 120.0F, column_w, 1.0F, 5.0F);
}

void draw_enhanced_workbench(SDL_Renderer* r, int width, int height,
                             const WorkbenchState& s,
                             const InteractionState& interaction) {
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);
    draw_floor(r, w, h);
    draw_board(r, w, h, s);
    draw_interaction_overlay(r, w, h, s, interaction);
    draw_palette(r, w, h, s);
    draw_inspector(r, w, h, s);
    draw_top_bar(r, w, s);
    if (s.show_help) draw_enhanced_help_overlay(r, w, h);
    SDL_RenderPresent(r);
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
        "FormFactor Workbench prototype", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 1480, 900,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    WorkbenchState state;
    state.show_help = false;
    state.status = "READY - MOVE OVER THE BOARD TO PREVIEW A PART.";
    InteractionState interaction;
    update_window_title(window, state);

    bool running = true;
    while (running) {
        SDL_Event event{};
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
                continue;
            }

            if (event.type == SDL_MOUSEMOTION) {
                interaction.mouse_x = static_cast<float>(event.motion.x);
                interaction.mouse_y = static_cast<float>(event.motion.y);
                interaction.mouse_known = true;

                if (interaction.dragging) {
                    int width = 0;
                    int height = 0;
                    SDL_GetRendererOutputSize(renderer, &width, &height);
                    update_drag(state, interaction,
                                board_rect_for(static_cast<float>(width),
                                               static_cast<float>(height)));
                }
                continue;
            }

            if (event.type == SDL_KEYDOWN) {
                const SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE) {
                    if (!cancel_transient_action(state, interaction)) running = false;
                } else if (key == SDLK_DELETE || key == SDLK_BACKSPACE) {
                    interaction.dragging = false;
                    remove_focused_part(state);
                } else if (key == SDLK_h || key == SDLK_F1) {
                    state.show_help = !state.show_help;
                    state.status = state.show_help ? "HELP OPEN - H, F1, OR ESC CLOSES IT." : "HELP CLOSED.";
                } else if (key == SDLK_c) {
                    interaction.dragging = false;
                    clear_board(state);
                } else if (key == SDLK_v) {
                    run_validation(state);
                } else if (key == SDLK_TAB) {
                    cycle_focus(state, (event.key.keysym.mod & KMOD_SHIFT) ? -1 : 1);
                } else if (key == SDLK_UP || key == SDLK_LEFT) {
                    browse_focus(state, -1);
                } else if (key == SDLK_DOWN || key == SDLK_RIGHT) {
                    browse_focus(state, 1);
                } else if (key == SDLK_RETURN || key == SDLK_SPACE) {
                    activate_focus(state);
                }
                update_window_title(window, state);
                continue;
            }

            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                if (interaction.dragging) {
                    if (interaction.drag_moved && interaction.drag_index < state.parts.size()) {
                        state.status = std::string("MOVED ") +
                                       reference_for(state.parts, interaction.drag_index) +
                                       ". WIRES AND SCHEMATIC FOLLOWED IT.";
                    }
                    interaction.dragging = false;
                    interaction.drag_moved = false;
                    update_window_title(window, state);
                }
                continue;
            }

            if (event.type != SDL_MOUSEBUTTONDOWN) continue;

            int width = 0;
            int height = 0;
            SDL_GetRendererOutputSize(renderer, &width, &height);
            const float x = static_cast<float>(event.button.x);
            const float y = static_cast<float>(event.button.y);
            const float w = static_cast<float>(width);
            const float h = static_cast<float>(height);
            const RectF board = board_rect_for(w, h);
            interaction.mouse_x = x;
            interaction.mouse_y = y;
            interaction.mouse_known = true;

            if (state.show_help) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    state.show_help = false;
                    state.status = "HELP CLOSED - BUILD CONTROLS ACTIVE.";
                    update_window_title(window, state);
                }
                continue;
            }

            if (event.button.button == SDL_BUTTON_LEFT) {
                if (contains(help_button_for(w), x, y)) {
                    state.focus = FocusZone::Help;
                    state.show_help = true;
                    state.status = "HELP OPEN - H, F1, OR ESC CLOSES IT.";
                } else if (contains(validate_button_for(w), x, y)) {
                    state.focus = FocusZone::Validate;
                    run_validation(state);
                } else if (contains(clear_button_for(w), x, y)) {
                    state.focus = FocusZone::Clear;
                    interaction.dragging = false;
                    clear_board(state);
                } else if (click_compatible_target(state, x, y, w, h)) {
                    // Inspector connection shortcut handled by the existing prototype logic.
                } else if (!pick_palette(state, x, y, w)) {
                    std::size_t index = 0;
                    if (find_part(state, x, y, index)) {
                        begin_drag(state, interaction, index, x, y);
                    } else if (contains(board, x, y)) {
                        place_part_snapped(state, x, y, board);
                    }
                }
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                std::size_t index = 0;
                if (find_part(state, x, y, index)) connect_part(state, index);
            }
            update_window_title(window, state);
        }

        int width = 0;
        int height = 0;
        SDL_GetRendererOutputSize(renderer, &width, &height);
        draw_enhanced_workbench(renderer, width, height, state, interaction);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
