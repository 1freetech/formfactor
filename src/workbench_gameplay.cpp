#define main formfactor_legacy_main
#include "workbench.cpp"
#undef main

namespace {

constexpr std::size_t kHistoryLimit = 64;
constexpr float kNudgeStep = 12.0F;

struct BoardSnapshot {
    std::vector<PlacedPart> parts;
    std::vector<Wire> wires;
    FocusZone focus{FocusZone::Palette};
    std::size_t board_focus{0};
};

struct EditHistory {
    std::vector<BoardSnapshot> undo;
    std::vector<BoardSnapshot> redo;
};

struct InteractionState {
    float mouse_x{0.0F};
    float mouse_y{0.0F};
    bool mouse_known{false};
    bool dragging{false};
    std::size_t drag_index{0};
    float drag_offset_x{0.0F};
    float drag_offset_y{0.0F};
    RectF drag_origin{};
    BoardSnapshot drag_snapshot{};
    bool drag_snapshot_valid{false};
};

BoardSnapshot capture_board(const WorkbenchState& s) {
    return {s.parts, s.wires, s.focus, s.board_focus};
}

void push_limited(std::vector<BoardSnapshot>& stack, const BoardSnapshot& snapshot) {
    if (stack.size() >= kHistoryLimit) stack.erase(stack.begin());
    stack.push_back(snapshot);
}

void record_edit(EditHistory& history, const BoardSnapshot& before) {
    push_limited(history.undo, before);
    history.redo.clear();
}

void restore_board(WorkbenchState& s, const BoardSnapshot& snapshot,
                   const std::string& message) {
    s.parts = snapshot.parts;
    s.wires = snapshot.wires;
    s.has_wire_start = false;
    s.wire_start = 0;
    s.validation_ran = false;
    s.validation_passed = false;
    s.board_focus = snapshot.board_focus;
    s.focus = snapshot.focus;

    if (s.parts.empty()) {
        s.board_focus = 0;
        if (s.focus == FocusZone::BoardPart) s.focus = FocusZone::Palette;
    } else {
        s.board_focus = std::min(s.board_focus, s.parts.size() - 1);
    }
    s.status = message;
}

bool undo_edit(WorkbenchState& s, EditHistory& history) {
    if (history.undo.empty()) {
        s.status = "NOTHING TO UNDO.";
        return false;
    }
    const BoardSnapshot current = capture_board(s);
    const BoardSnapshot previous = history.undo.back();
    history.undo.pop_back();
    push_limited(history.redo, current);
    restore_board(s, previous, "UNDO - LAST BOARD EDIT RESTORED. RETEST WHEN READY.");
    return true;
}

bool redo_edit(WorkbenchState& s, EditHistory& history) {
    if (history.redo.empty()) {
        s.status = "NOTHING TO REDO.";
        return false;
    }
    const BoardSnapshot current = capture_board(s);
    const BoardSnapshot next = history.redo.back();
    history.redo.pop_back();
    push_limited(history.undo, current);
    restore_board(s, next, "REDO - BOARD EDIT RESTORED. RETEST WHEN READY.");
    return true;
}

bool rects_overlap(const RectF& a, const RectF& b) {
    return a.x < b.x + b.w && a.x + a.w > b.x &&
           a.y < b.y + b.h && a.y + a.h > b.y;
}

bool rects_equal(const RectF& a, const RectF& b) {
    constexpr float epsilon = 0.1F;
    return std::fabs(a.x - b.x) <= epsilon &&
           std::fabs(a.y - b.y) <= epsilon &&
           std::fabs(a.w - b.w) <= epsilon &&
           std::fabs(a.h - b.h) <= epsilon;
}

bool overlaps_any(const WorkbenchState& s, const RectF& candidate,
                  std::size_t ignored_index = static_cast<std::size_t>(-1)) {
    for (std::size_t i = 0; i < s.parts.size(); ++i) {
        if (i == ignored_index) continue;
        if (rects_overlap(candidate, s.parts[i].rect)) return true;
    }
    return false;
}

bool focused_part_overlaps(const WorkbenchState& s, std::size_t index) {
    return index < s.parts.size() && overlaps_any(s, s.parts[index].rect, index);
}

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

bool pick_same_part(WorkbenchState& s) {
    if (!focused_board_part_valid(s)) {
        s.status = "FOCUS A BOARD PART BEFORE USING PICK SAME PART.";
        return false;
    }

    const std::size_t index = s.board_focus;
    const PartKind picked = s.parts[index].kind;
    s.selected_kind = picked;
    s.palette_focus = static_cast<std::size_t>(picked);
    s.focus = FocusZone::Palette;
    s.status = std::string("PICKED ") + info(picked).code + " " + info(picked).name +
               " FROM " + reference_for(s.parts, index) +
               ". CLICK AN OPEN BOARD SPOT TO PLACE THE SAME PART.";
    return true;
}

bool place_part_snapped(WorkbenchState& s, EditHistory& history,
                        float x, float y, const RectF& board) {
    const RectF q = snapped_part_rect(s.selected_kind, x, y, board);
    if (overlaps_any(s, q)) {
        s.status = "BLOCKED - PARTS CANNOT OVERLAP. PICK AN OPEN SPOT.";
        return false;
    }

    record_edit(history, capture_board(s));
    s.parts.push_back({s.selected_kind, q});
    s.board_focus = s.parts.size() - 1;
    s.focus = FocusZone::BoardPart;
    mark_board_changed(s);
    s.status = std::string("PLACED ") + reference_for(s.parts, s.board_focus) +
               ". CTRL+Z UNDOS IT. DRAG TO MOVE.";
    return true;
}

void remove_focused_part(WorkbenchState& s, EditHistory& history) {
    if (!focused_board_part_valid(s)) {
        s.status = "FOCUS A BOARD PART BEFORE DELETE.";
        return;
    }

    record_edit(history, capture_board(s));
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

    s.parts.erase(s.parts.begin() + static_cast<std::ptrdiff_t>(removed));
    mark_board_changed(s);

    if (s.parts.empty()) {
        s.board_focus = 0;
        s.focus = FocusZone::Palette;
    } else {
        s.board_focus = std::min(removed, s.parts.size() - 1);
        s.focus = FocusZone::BoardPart;
    }

    s.status = "REMOVED " + removed_name + ". CTRL+Z RESTORES IT.";
}

void clear_board_with_history(WorkbenchState& s, EditHistory& history) {
    if (s.parts.empty() && s.wires.empty()) {
        s.status = "BOARD IS ALREADY CLEAR.";
        return;
    }
    record_edit(history, capture_board(s));
    clear_board(s);
    s.status = "BOARD CLEARED. CTRL+Z RESTORES THE LAST BOARD.";
}

void connect_part_with_history(WorkbenchState& s, EditHistory& history,
                               std::size_t index) {
    const BoardSnapshot before = capture_board(s);
    const std::size_t wire_count = s.wires.size();
    connect_part(s, index);
    if (s.wires.size() > wire_count) record_edit(history, before);
}

bool click_compatible_target_with_history(WorkbenchState& s, EditHistory& history,
                                          float x, float y, float width, float height) {
    const RectF panel = inspector_rect_for(width, height);
    const std::vector<std::size_t> targets = compatible_targets(s);
    const std::size_t shown = std::min<std::size_t>(targets.size(), 3);
    for (std::size_t row = 0; row < shown; ++row) {
        if (!contains(target_button_rect(panel, row), x, y)) continue;
        if (!s.has_wire_start && focused_board_part_valid(s)) {
            connect_part_with_history(s, history, s.board_focus);
        }
        if (s.has_wire_start) connect_part_with_history(s, history, targets[row]);
        return true;
    }
    return false;
}

void activate_focus_with_history(WorkbenchState& s, EditHistory& history) {
    switch (s.focus) {
        case FocusZone::Palette:
            s.selected_kind = static_cast<PartKind>(s.palette_focus);
            s.status = std::string("SELECTED ") + info(s.selected_kind).code + " " +
                       info(s.selected_kind).name + ". CLICK BOARD TO PLACE.";
            break;
        case FocusZone::BoardPart:
            if (!s.parts.empty()) {
                connect_part_with_history(s, history,
                                          std::min(s.board_focus, s.parts.size() - 1));
            }
            break;
        case FocusZone::Validate:
            run_validation(s);
            break;
        case FocusZone::Clear:
            clear_board_with_history(s, history);
            break;
        case FocusZone::Help:
            s.show_help = !s.show_help;
            break;
    }
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

    s.status = std::string("DISCONNECTED ") + reference_for(s.parts, index) +
               " FROM " + std::to_string(connected_count) +
               (connected_count == 1 ? " WIRE. " : " WIRES. ") +
               "RIGHT CLICK TO REWIRE. CTRL+Z RESTORES IT.";
    return true;
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
        interaction.drag_snapshot_valid = false;
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

void cancel_drag_silently(WorkbenchState& s, InteractionState& interaction) {
    if (interaction.dragging && interaction.drag_index < s.parts.size()) {
        s.parts[interaction.drag_index].rect = interaction.drag_origin;
    }
    interaction.dragging = false;
    interaction.drag_snapshot_valid = false;
}

void begin_drag(WorkbenchState& s, InteractionState& interaction,
                std::size_t index, float x, float y) {
    if (index >= s.parts.size()) return;
    s.focus = FocusZone::BoardPart;
    s.board_focus = index;
    interaction.dragging = true;
    interaction.drag_index = index;
    interaction.drag_origin = s.parts[index].rect;
    interaction.drag_snapshot = capture_board(s);
    interaction.drag_snapshot_valid = true;
    interaction.drag_offset_x = x - s.parts[index].rect.x;
    interaction.drag_offset_y = y - s.parts[index].rect.y;
    s.status = std::string("SELECTED ") + reference_for(s.parts, index) +
               ". DRAG TO MOVE. RED MEANS BLOCKED.";
}

void update_drag(WorkbenchState& s, InteractionState& interaction, const RectF& board) {
    if (!interaction.dragging || interaction.drag_index >= s.parts.size()) return;

    RectF q = s.parts[interaction.drag_index].rect;
    const float raw_x = interaction.mouse_x - interaction.drag_offset_x;
    const float raw_y = interaction.mouse_y - interaction.drag_offset_y;
    q.x = snap_to_half_grid(raw_x, board.x);
    q.y = snap_to_half_grid(raw_y, board.y);
    q = clamp_part_rect(q, board);

    if (!rects_equal(s.parts[interaction.drag_index].rect, q)) {
        s.parts[interaction.drag_index].rect = q;
        s.validation_ran = false;
        s.validation_passed = false;
    }
}

void finish_drag(WorkbenchState& s, EditHistory& history, InteractionState& interaction) {
    if (!interaction.dragging || interaction.drag_index >= s.parts.size()) return;

    const std::size_t index = interaction.drag_index;
    const bool moved = !rects_equal(s.parts[index].rect, interaction.drag_origin);
    const bool blocked = focused_part_overlaps(s, index);

    if (blocked) {
        s.parts[index].rect = interaction.drag_origin;
        s.status = "BLOCKED - PARTS CANNOT OVERLAP. MOVE RETURNED TO LAST GOOD SPOT.";
    } else if (moved) {
        if (interaction.drag_snapshot_valid) record_edit(history, interaction.drag_snapshot);
        s.status = std::string("MOVED ") + reference_for(s.parts, index) +
                   ". CTRL+Z UNDOS THE MOVE.";
    }

    interaction.dragging = false;
    interaction.drag_snapshot_valid = false;
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

    if (interaction.dragging && interaction.drag_index < s.parts.size()) {
        const RectF& p = s.parts[interaction.drag_index].rect;
        const bool blocked = focused_part_overlaps(s, interaction.drag_index);
        if (blocked) set_color(r, 255, 70, 70, 245);
        else set_color(r, 255, 225, 70, 235);
        draw_double_rect(r, {p.x - 5.0F, p.y - 5.0F, p.w + 10.0F, p.h + 10.0F});
        if (blocked) {
            draw_text(r, "BLOCKED", p.x,
                      std::max(board.y + 5.0F, p.y - 18.0F), 0.8F);
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
    const bool blocked = overlaps_any(s, ghost);
    if (blocked) set_color(r, 255, 70, 70, 48);
    else set_color(r, 57, 255, 20, 46);
    fill_rect(r, ghost);
    if (blocked) set_color(r, 255, 70, 70, 235);
    else set_color(r, 57, 255, 20, 220);
    draw_double_rect(r, ghost);
    draw_schematic_symbol(r, s.selected_kind,
                          {ghost.x + 5.0F, ghost.y + 5.0F,
                           std::max(18.0F, ghost.w - 10.0F),
                           std::max(18.0F, ghost.h - 10.0F)}, true);

    const float label_y = std::max(board.y + 5.0F, ghost.y - 17.0F);
    if (blocked) set_color(r, 255, 100, 100);
    else set_color(r, 225, 239, 232);
    draw_text(r, blocked ? "BLOCKED - OVERLAP" : "CLICK TO PLACE",
              ghost.x, label_y, 0.75F);
}

void draw_controls_help_overlay(SDL_Renderer* r, float w, float h) {
    draw_help_overlay(r, w, h);
    const RectF q{w * 0.13F, h * 0.10F, w * 0.74F, h * 0.78F};
    const float left = q.x + 30.0F;
    const float right = q.x + q.w * 0.52F;
    const float column_w = q.w * 0.42F;

    set_color(r, 3, 6, 7, 252);
    fill_rect(r, {left - 4.0F, q.y + 340.0F, column_w + 8.0F, 190.0F});
    fill_rect(r, {right - 4.0F, q.y + 82.0F, column_w + 8.0F, 330.0F});

    set_color(r, 57, 255, 20);
    draw_text(r, "MOUSE", left, q.y + 350.0F, 1.35F);
    set_color(r, 210, 221, 223);
    draw_wrapped_text(
        r,
        "MOVE OVER EMPTY BOARD SPACE TO PREVIEW A SNAPPED PART. GREEN MEANS OPEN. RED MEANS OVERLAP AND THE DROP IS BLOCKED. CLICK TO PLACE. CLICK AND DRAG A PLACED PART TO MOVE IT. RIGHT CLICK ONE PART THEN ANOTHER TO CONNECT.",
        left, q.y + 378.0F, column_w, 1.0F, 5.0F);

    set_color(r, 57, 255, 20);
    draw_text(r, "KEYBOARD", right, q.y + 92.0F, 1.45F);
    set_color(r, 224, 232, 234);
    draw_wrapped_text(
        r,
        "CTRL+Z: UNDO. CTRL+Y OR CTRL+SHIFT+Z: REDO. CTRL+D: DUPLICATE FOCUSED PART. E: PICK SAME PART TYPE FROM FOCUSED PART. SHIFT+ARROWS: NUDGE ONE GRID STEP. X: DISCONNECT FOCUSED PART WITHOUT DELETING IT. DELETE OR BACKSPACE: REMOVE PART. V: TEST. C: CLEAR. H OR F1: HELP. ESC: CANCEL ACTIVE ACTION FIRST, THEN EXIT.",
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
    state.status = "READY - E PICKS SAME PART. SHIFT+ARROWS NUDGE. CTRL+D DUPLICATES. X DISCONNECTS.";
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
                } else if (key == SDLK_e && !command) {
                    cancel_drag_silently(state, interaction);
                    pick_same_part(state);
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
