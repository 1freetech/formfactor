#define main formfactor_gameplay_main
#include "workbench_gameplay.cpp"
#undef main

namespace {

constexpr float kNudgeStep = 12.0F;

bool focused_board_part_valid(const WorkbenchState& s) {
    return s.focus == FocusZone::BoardPart && !s.parts.empty() &&
           s.board_focus < s.parts.size();
}

void mark_board_changed(WorkbenchState& s) {
    s.validation_ran = false;
    s.validation_passed = false;
    s.has_wire_start = false;
    s.wire_start = 0;
}

bool nudge_focused_part(WorkbenchState& s, EditHistory& history,
                        const RectF& board, int dx, int dy) {
    if (!focused_board_part_valid(s)) {
        s.status = "FOCUS A BOARD PART BEFORE NUDGING IT.";
        return false;
    }

    const std::size_t index = s.board_focus;
    const RectF original = s.parts[index].rect;
    RectF candidate = original;
    candidate.x += static_cast<float>(dx) * kNudgeStep;
    candidate.y += static_cast<float>(dy) * kNudgeStep;
    candidate = clamp_part_rect(candidate, board);

    if (rects_equal(candidate, original)) {
        s.status = "NUDGE BLOCKED - PART IS AT THE BOARD EDGE.";
        return false;
    }
    if (overlaps_any(s, candidate, index)) {
        s.status = "NUDGE BLOCKED - ANOTHER PART IS IN THE WAY.";
        return false;
    }

    record_edit(history, capture_board(s));
    s.parts[index].rect = candidate;
    mark_board_changed(s);
    s.status = std::string("NUDGED ") + reference_for(s.parts, index) +
               ". SHIFT+ARROWS MOVE ONE GRID STEP. CTRL+Z UNDOS IT.";
    return true;
}

bool duplicate_focused_part(WorkbenchState& s, EditHistory& history,
                            const RectF& board) {
    if (!focused_board_part_valid(s)) {
        s.status = "FOCUS A BOARD PART BEFORE DUPLICATING IT.";
        return false;
    }

    const std::size_t source_index = s.board_focus;
    const PlacedPart source = s.parts[source_index];

    for (int radius = 1; radius <= 12; ++radius) {
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                if (std::max(std::abs(dx), std::abs(dy)) != radius) continue;

                RectF candidate = source.rect;
                candidate.x += static_cast<float>(dx) * kNudgeStep;
                candidate.y += static_cast<float>(dy) * kNudgeStep;
                const RectF clamped = clamp_part_rect(candidate, board);
                if (!rects_equal(clamped, candidate)) continue;
                if (overlaps_any(s, candidate)) continue;

                record_edit(history, capture_board(s));
                s.parts.push_back({source.kind, candidate});
                s.board_focus = s.parts.size() - 1;
                s.focus = FocusZone::BoardPart;
                mark_board_changed(s);
                s.status = std::string("DUPLICATED ") +
                           reference_for(s.parts, source_index) + " AS " +
                           reference_for(s.parts, s.board_focus) +
                           ". WIRES WERE NOT COPIED. CTRL+Z UNDOS IT.";
                return true;
            }
        }
    }

    s.status = "DUPLICATE BLOCKED - NO OPEN GRID SPOT IS CLOSE ENOUGH.";
    return false;
}

bool disconnect_focused_part(WorkbenchState& s, EditHistory& history) {
    if (!focused_board_part_valid(s)) {
        s.status = "FOCUS A BOARD PART BEFORE DISCONNECTING IT.";
        return false;
    }

    const std::size_t index = s.board_focus;
    const std::size_t before_count = s.wires.size();
    const std::size_t connected_count = static_cast<std::size_t>(std::count_if(
        s.wires.begin(), s.wires.end(), [index](const Wire& wire) {
            return wire.from == index || wire.to == index;
        }));

    if (connected_count == 0) {
        s.status = std::string("NO WIRES TO DISCONNECT FROM ") +
                   reference_for(s.parts, index) + ".";
        return false;
    }

    record_edit(history, capture_board(s));
    s.wires.erase(
        std::remove_if(s.wires.begin(), s.wires.end(), [index](const Wire& wire) {
            return wire.from == index || wire.to == index;
        }),
        s.wires.end());
    mark_board_changed(s);

    const std::size_t removed = before_count - s.wires.size();
    s.status = std::string("DISCONNECTED ") + reference_for(s.parts, index) +
               " FROM " + std::to_string(removed) +
               (removed == 1 ? " WIRE. " : " WIRES. ") +
               "RIGHT CLICK TO REWIRE. CTRL+Z RESTORES IT.";
    return true;
}

void draw_controls_help_overlay(SDL_Renderer* r, float w, float h) {
    draw_enhanced_help_overlay(r, w, h);
    const RectF q{w * 0.13F, h * 0.10F, w * 0.74F, h * 0.78F};
    const float right = q.x + q.w * 0.52F;
    const float column_w = q.w * 0.42F;

    set_color(r, 3, 6, 7, 252);
    fill_rect(r, {right - 4.0F, q.y + 82.0F, column_w + 8.0F, 305.0F});

    set_color(r, 57, 255, 20);
    draw_text(r, "KEYBOARD", right, q.y + 92.0F, 1.45F);
    set_color(r, 224, 232, 234);
    draw_wrapped_text(
        r,
        "CTRL+Z: UNDO. CTRL+Y OR CTRL+SHIFT+Z: REDO. CTRL+D: DUPLICATE THE FOCUSED PART INTO THE NEAREST OPEN GRID SPOT. SHIFT+ARROWS: NUDGE THE FOCUSED PART ONE GRID STEP. X: DISCONNECT ALL WIRES FROM THE FOCUSED PART WITHOUT DELETING IT. DELETE OR BACKSPACE: REMOVE PART. V: TEST. C: CLEAR. H OR F1: HELP. ESC: CANCEL ACTIVE ACTION FIRST, THEN EXIT.",
        right, q.y + 120.0F, column_w, 1.0F, 5.0F);
}

void draw_controls_workbench(SDL_Renderer* r, int width, int height,
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
    if (s.show_help) draw_controls_help_overlay(r, w, h);
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
        "FormFactor Workbench", SDL_WINDOWPOS_CENTERED,
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
    state.status = "READY - SHIFT+ARROWS NUDGE. CTRL+D DUPLICATES. X DISCONNECTS.";
    InteractionState interaction;
    EditHistory history;
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
                const Uint16 mods = event.key.keysym.mod;
                const bool command = (mods & KMOD_CTRL) != 0 || (mods & KMOD_GUI) != 0;
                const bool shift = (mods & KMOD_SHIFT) != 0;
                const bool undo_key = command && key == SDLK_z && !shift;
                const bool redo_key = command && (key == SDLK_y || (key == SDLK_z && shift));
                const bool duplicate_key = command && key == SDLK_d;
                const bool nudge_key = shift &&
                    (key == SDLK_UP || key == SDLK_DOWN ||
                     key == SDLK_LEFT || key == SDLK_RIGHT) &&
                    focused_board_part_valid(state);

                if (undo_key) {
                    cancel_drag_silently(state, interaction);
                    undo_edit(state, history);
                } else if (redo_key) {
                    cancel_drag_silently(state, interaction);
                    redo_edit(state, history);
                } else if (duplicate_key) {
                    cancel_drag_silently(state, interaction);
                    int width = 0;
                    int height = 0;
                    SDL_GetRendererOutputSize(renderer, &width, &height);
                    duplicate_focused_part(
                        state, history,
                        board_rect_for(static_cast<float>(width),
                                       static_cast<float>(height)));
                } else if (nudge_key) {
                    cancel_drag_silently(state, interaction);
                    int width = 0;
                    int height = 0;
                    SDL_GetRendererOutputSize(renderer, &width, &height);
                    const RectF board = board_rect_for(static_cast<float>(width),
                                                       static_cast<float>(height));
                    int dx = 0;
                    int dy = 0;
                    if (key == SDLK_LEFT) dx = -1;
                    else if (key == SDLK_RIGHT) dx = 1;
                    else if (key == SDLK_UP) dy = -1;
                    else if (key == SDLK_DOWN) dy = 1;
                    nudge_focused_part(state, history, board, dx, dy);
                } else if (key == SDLK_x && !command) {
                    cancel_drag_silently(state, interaction);
                    disconnect_focused_part(state, history);
                } else if (key == SDLK_ESCAPE) {
                    if (!cancel_transient_action(state, interaction)) running = false;
                } else if (key == SDLK_DELETE || key == SDLK_BACKSPACE) {
                    cancel_drag_silently(state, interaction);
                    remove_focused_part(state, history);
                } else if (key == SDLK_h || key == SDLK_F1) {
                    state.show_help = !state.show_help;
                    state.status = state.show_help ?
                        "HELP OPEN - H, F1, OR ESC CLOSES IT." : "HELP CLOSED.";
                } else if (key == SDLK_c) {
                    cancel_drag_silently(state, interaction);
                    clear_board_with_history(state, history);
                } else if (key == SDLK_v) {
                    run_validation(state);
                } else if (key == SDLK_TAB) {
                    cycle_focus(state, shift ? -1 : 1);
                } else if (key == SDLK_UP || key == SDLK_LEFT) {
                    browse_focus(state, -1);
                } else if (key == SDLK_DOWN || key == SDLK_RIGHT) {
                    browse_focus(state, 1);
                } else if (key == SDLK_RETURN || key == SDLK_SPACE) {
                    activate_focus_with_history(state, history);
                }
                update_window_title(window, state);
                continue;
            }

            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                if (interaction.dragging) {
                    finish_drag(state, history, interaction);
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
                    cancel_drag_silently(state, interaction);
                    clear_board_with_history(state, history);
                } else if (click_compatible_target_with_history(state, history, x, y, w, h)) {
                    // Inspector connection shortcut handled with history above.
                } else if (!pick_palette(state, x, y, w)) {
                    std::size_t index = 0;
                    if (find_part(state, x, y, index)) {
                        begin_drag(state, interaction, index, x, y);
                    } else if (contains(board, x, y)) {
                        place_part_snapped(state, history, x, y, board);
                    }
                }
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                std::size_t index = 0;
                if (find_part(state, x, y, index)) {
                    connect_part_with_history(state, history, index);
                }
            }
            update_window_title(window, state);
        }

        int width = 0;
        int height = 0;
        SDL_GetRendererOutputSize(renderer, &width, &height);
        draw_controls_workbench(renderer, width, height, state, interaction);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
