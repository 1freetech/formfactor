#include <SDL2/SDL.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {

struct RectF { float x; float y; float w; float h; };
enum class PartKind : std::size_t { Resistor = 0, Capacitor, Led, Chip, Power, Connector };
enum class FocusZone { Palette, BoardPart, Validate, Clear };
struct PlacedPart { PartKind kind; RectF rect; };
struct Wire { std::size_t from; std::size_t to; };

struct PartInfo {
    const char* code;
    const char* name;
    const char* description;
    const char* pairs;
};

constexpr std::array<PartInfo, 6> kPartInfo{{
    {"R", "RESISTOR", "LIMITS CURRENT AND CREATES A CONTROLLED VOLTAGE DROP.", "PWR, LED, IC, J, C"},
    {"C", "CAPACITOR", "STORES CHARGE AND HELPS SMOOTH OR FILTER VOLTAGE CHANGES.", "PWR, R, IC, J"},
    {"LED", "LED", "LIGHTS WHEN CURRENT FLOWS THROUGH IT IN THE CORRECT DIRECTION.", "PWR, R, IC, J"},
    {"IC", "CHIP", "REPRESENTS AN INTEGRATED CIRCUIT THAT USES POWER AND SIGNALS.", "PWR, R, C, LED, J"},
    {"PWR", "POWER", "SUPPLIES ELECTRICAL ENERGY TO THE TRAINING BOARD.", "R, C, LED, IC, J"},
    {"J", "CONNECTOR", "GIVES POWER OR SIGNALS A PHYSICAL PATH INTO OR OUT OF THE BOARD.", "PWR, R, C, LED, IC"},
}};

const PartInfo& info(PartKind kind) {
    return kPartInfo[static_cast<std::size_t>(kind)];
}

void set_color(SDL_Renderer* r, std::uint8_t red, std::uint8_t green,
               std::uint8_t blue, std::uint8_t alpha = 255) {
    SDL_SetRenderDrawColor(r, red, green, blue, alpha);
}

void fill_rect(SDL_Renderer* r, const RectF& q) {
    const SDL_FRect x{q.x, q.y, q.w, q.h};
    SDL_RenderFillRectF(r, &x);
}

void draw_rect(SDL_Renderer* r, const RectF& q) {
    const SDL_FRect x{q.x, q.y, q.w, q.h};
    SDL_RenderDrawRectF(r, &x);
}

bool contains(const RectF& q, float x, float y) {
    return x >= q.x && x <= q.x + q.w && y >= q.y && y <= q.y + q.h;
}

const char* glyph(char ch) {
    switch (static_cast<char>(std::toupper(static_cast<unsigned char>(ch)))) {
        case 'A': return "01110100011000111111100011000110001";
        case 'B': return "11110100011000111110100011000111110";
        case 'C': return "01111100001000010000100001000001111";
        case 'D': return "11110100011000110001100011000111110";
        case 'E': return "11111100001000011110100001000011111";
        case 'F': return "11111100001000011110100001000010000";
        case 'G': return "01111100001000010111100011000101111";
        case 'H': return "10001100011000111111100011000110001";
        case 'I': return "11111001000010000100001000010011111";
        case 'J': return "00111000100001000010100101001001100";
        case 'K': return "10001100101010011000101001001010001";
        case 'L': return "10000100001000010000100001000011111";
        case 'M': return "10001110111010110101100011000110001";
        case 'N': return "10001110011010110011100011000110001";
        case 'O': return "01110100011000110001100011000101110";
        case 'P': return "11110100011000111110100001000010000";
        case 'Q': return "01110100011000110001101011001001101";
        case 'R': return "11110100011000111110101001001010001";
        case 'S': return "01111100001000001110000010000111110";
        case 'T': return "11111001000010000100001000010000100";
        case 'U': return "10001100011000110001100011000101110";
        case 'V': return "10001100011000110001100010101000100";
        case 'W': return "10001100011000110101101011010101010";
        case 'X': return "10001100010101000100010101000110001";
        case 'Y': return "10001100010101000100001000010000100";
        case 'Z': return "11111000010001000100010001000011111";
        case '0': return "01110100011001110101110011000101110";
        case '1': return "00100011000010000100001000010001110";
        case '2': return "01110100010000100010001000100011111";
        case '3': return "11110000010000101110000010000111110";
        case '4': return "00010001100101010010111110001000010";
        case '5': return "11111100001000011110000010000111110";
        case '6': return "01110100001000011110100011000101110";
        case '7': return "11111000010001000100010000100001000";
        case '8': return "01110100011000101110100011000101110";
        case '9': return "01110100011000101111000010000101110";
        case ':': return "00000001000010000000001000010000000";
        case '.': return "00000000000000000000000000010000100";
        case ',': return "00000000000000000000001000010001000";
        case '-': return "00000000000000011111000000000000000";
        case '+': return "00000001000010011111001000010000000";
        case '/': return "00001000100010001000100001000000000";
        case '?': return "01110100010000100110001000000000100";
        case '!': return "00100001000010000100001000000000100";
        case '(': return "00010001000100001000010000010000010";
        case ')': return "01000001000001000010000100010001000";
        case '[': return "01110010000100001000010000100001110";
        case ']': return "01110000100001000010000100001001110";
        case '|': return "00100001000010000100001000010000100";
        case '>': return "01000001000001000001000100010001000";
        case '<': return "00010001000100010000010000010000010";
        case '=': return "00000000001111100000111110000000000";
        case ' ': return "00000000000000000000000000000000000";
        default: return "11111100011010110101100011111100000";
    }
}

void draw_char(SDL_Renderer* r, char ch, float x, float y, float scale) {
    const char* bits = glyph(ch);
    for (int row = 0; row < 7; ++row) {
        for (int col = 0; col < 5; ++col) {
            if (bits[row * 5 + col] == '1') {
                fill_rect(r, {x + static_cast<float>(col) * scale,
                              y + static_cast<float>(row) * scale, scale, scale});
            }
        }
    }
}

void draw_text(SDL_Renderer* r, const std::string& text, float x, float y,
               float scale) {
    float cursor = x;
    for (char ch : text) {
        draw_char(r, ch, cursor, y, scale);
        cursor += 6.0F * scale;
    }
}

void draw_wrapped_text(SDL_Renderer* r, const std::string& text, float x,
                       float y, float max_width, float scale, float line_gap = 3.0F) {
    const int max_chars = std::max(1, static_cast<int>(max_width / (6.0F * scale)));
    std::string line;
    std::string word;
    float line_y = y;
    auto flush_word = [&]() {
        if (word.empty()) return;
        if (!line.empty() && static_cast<int>(line.size() + 1 + word.size()) > max_chars) {
            draw_text(r, line, x, line_y, scale);
            line_y += 7.0F * scale + line_gap;
            line.clear();
        }
        if (!line.empty()) line += ' ';
        line += word;
        word.clear();
    };
    for (char ch : text) {
        if (ch == ' ') flush_word();
        else word += ch;
    }
    flush_word();
    if (!line.empty()) draw_text(r, line, x, line_y, scale);
}

RectF part_rect(PartKind kind, float x, float y) {
    switch (kind) {
        case PartKind::Resistor: return {x - 34, y - 13, 68, 26};
        case PartKind::Capacitor: return {x - 22, y - 24, 44, 48};
        case PartKind::Led: return {x - 20, y - 20, 40, 40};
        case PartKind::Chip: return {x - 46, y - 30, 92, 60};
        case PartKind::Power: return {x - 36, y - 26, 72, 52};
        case PartKind::Connector: return {x - 24, y - 42, 48, 84};
    }
    return {x - 25, y - 20, 50, 40};
}

void part_color(SDL_Renderer* r, PartKind kind) {
    switch (kind) {
        case PartKind::Resistor: set_color(r, 190, 132, 69); break;
        case PartKind::Capacitor: set_color(r, 56, 122, 209); break;
        case PartKind::Led: set_color(r, 57, 255, 20); break;
        case PartKind::Chip: set_color(r, 35, 39, 44); break;
        case PartKind::Power: set_color(r, 196, 58, 58); break;
        case PartKind::Connector: set_color(r, 201, 210, 214); break;
    }
}

void draw_raised_box(SDL_Renderer* r, const RectF& q, bool focused) {
    set_color(r, 4, 6, 7, 180);
    fill_rect(r, {q.x + 7, q.y + 8, q.w, q.h});
    set_color(r, 20, 27, 29);
    fill_rect(r, q);
    set_color(r, focused ? 57 : 86, focused ? 255 : 104, focused ? 20 : 108);
    draw_rect(r, q);
    SDL_RenderDrawLineF(r, q.x + 1, q.y + 1, q.x + q.w - 1, q.y + 1);
    SDL_RenderDrawLineF(r, q.x + 1, q.y + 1, q.x + 1, q.y + q.h - 1);
    set_color(r, 5, 10, 11);
    SDL_RenderDrawLineF(r, q.x, q.y + q.h, q.x + q.w, q.y + q.h);
    SDL_RenderDrawLineF(r, q.x + q.w, q.y, q.x + q.w, q.y + q.h);
}

void draw_part(SDL_Renderer* r, const PlacedPart& p, bool selected) {
    const RectF shadow{p.rect.x + 6, p.rect.y + 7, p.rect.w, p.rect.h};
    set_color(r, 3, 5, 6, 190);
    fill_rect(r, shadow);
    part_color(r, p.kind);
    fill_rect(r, p.rect);
    set_color(r, selected ? 255 : 230, selected ? 225 : 235, selected ? 70 : 238);
    draw_rect(r, p.rect);
    set_color(r, 255, 255, 255, 95);
    SDL_RenderDrawLineF(r, p.rect.x + 2, p.rect.y + 2, p.rect.x + p.rect.w - 2, p.rect.y + 2);
    const float cy = p.rect.y + p.rect.h * 0.5F;
    set_color(r, 224, 164, 50);
    fill_rect(r, {p.rect.x - 7, cy - 3, 7, 6});
    fill_rect(r, {p.rect.x + p.rect.w, cy - 3, 7, 6});
    set_color(r, 12, 13, 14);
    const std::string code = info(p.kind).code;
    const float scale = code.size() <= 2 ? 2.0F : 1.3F;
    draw_text(r, code, p.rect.x + 7, p.rect.y + 7, scale);
}

struct WorkbenchState {
    PartKind selected_kind{PartKind::Resistor};
    std::vector<PlacedPart> parts;
    std::vector<Wire> wires;
    FocusZone focus{FocusZone::Palette};
    std::size_t palette_focus{0};
    std::size_t board_focus{0};
    bool has_wire_start{false};
    std::size_t wire_start{0};
    bool validation_ran{false};
    bool validation_passed{false};
    bool show_help{true};
    std::string status{"READY - CHOOSE A PART."};
};

float palette_width_for(float w) { return std::min(280.0F, w * 0.21F); }
float inspector_width_for(float w) { return std::min(350.0F, w * 0.27F); }
RectF board_rect_for(float w, float h) {
    const float left = palette_width_for(w);
    const float right = inspector_width_for(w);
    const float margin = 28.0F;
    return {left + margin, 104.0F,
            std::max(360.0F, w - left - right - margin * 2.0F),
            std::max(280.0F, h - 104.0F - 54.0F)};
}
RectF inspector_rect_for(float w, float h) {
    const float iw = inspector_width_for(w);
    return {w - iw, 72.0F, iw, h - 72.0F};
}
RectF validate_button_for(float w) { return {w - inspector_width_for(w) - 150.0F, 18.0F, 130.0F, 34.0F}; }
RectF clear_button_for(float w) { return {w - inspector_width_for(w) - 290.0F, 18.0F, 124.0F, 34.0F}; }

bool is_load(PartKind kind) {
    return kind == PartKind::Resistor || kind == PartKind::Led ||
           kind == PartKind::Chip || kind == PartKind::Connector;
}

bool prototype_pair_allowed(PartKind a, PartKind b) {
    if (a == b) return false;
    if (a == PartKind::Power || b == PartKind::Power) return true;
    if (a == PartKind::Connector || b == PartKind::Connector) return true;
    if ((a == PartKind::Resistor && (b == PartKind::Led || b == PartKind::Chip || b == PartKind::Capacitor)) ||
        (b == PartKind::Resistor && (a == PartKind::Led || a == PartKind::Chip || a == PartKind::Capacitor))) return true;
    if ((a == PartKind::Chip && (b == PartKind::Led || b == PartKind::Capacitor)) ||
        (b == PartKind::Chip && (a == PartKind::Led || a == PartKind::Capacitor))) return true;
    return false;
}

std::vector<std::size_t> compatible_targets(const WorkbenchState& s) {
    std::vector<std::size_t> targets;
    std::size_t anchor = 0;
    if (s.has_wire_start && s.wire_start < s.parts.size()) anchor = s.wire_start;
    else if (s.focus == FocusZone::BoardPart && s.board_focus < s.parts.size()) anchor = s.board_focus;
    else return targets;
    for (std::size_t i = 0; i < s.parts.size(); ++i) {
        if (i == anchor) continue;
        if (prototype_pair_allowed(s.parts[anchor].kind, s.parts[i].kind)) targets.push_back(i);
    }
    return targets;
}

RectF target_button_rect(const RectF& panel, std::size_t row) {
    return {panel.x + 18.0F, panel.y + 374.0F + static_cast<float>(row) * 36.0F, panel.w - 36.0F, 28.0F};
}

void run_validation(WorkbenchState& s) {
    s.validation_ran = true;
    s.validation_passed = false;
    for (const Wire& wire : s.wires) {
        if (wire.from >= s.parts.size() || wire.to >= s.parts.size()) continue;
        const PartKind a = s.parts[wire.from].kind;
        const PartKind b = s.parts[wire.to].kind;
        if ((a == PartKind::Power && is_load(b)) ||
            (b == PartKind::Power && is_load(a))) {
            s.validation_passed = true;
            s.status = "TEST PASS - POWER IS WIRED TO A LOAD.";
            return;
        }
    }
    s.status = "TEST NEEDS POWER + A LOAD + A WIRE.";
}

void update_window_title(SDL_Window* w, const WorkbenchState& s) {
    std::string title = "FormFactor | TAB focus | ARROWS browse | ENTER use | mouse click | H help | ";
    title += s.status;
    SDL_SetWindowTitle(w, title.c_str());
}

void draw_floor(SDL_Renderer* r, float w, float h) {
    set_color(r, 8, 11, 13);
    SDL_RenderClear(r);
    const float horizon = 86.0F;
    set_color(r, 14, 19, 21);
    fill_rect(r, {0, horizon, w, h - horizon});
    set_color(r, 22, 32, 34);
    for (int i = 1; i <= 11; ++i) {
        const float t = static_cast<float>(i) / 11.0F;
        const float y = horizon + t * t * (h - horizon);
        SDL_RenderDrawLineF(r, 0, y, w, y);
    }
    const float vanishing_x = w * 0.56F;
    for (int i = -8; i <= 8; ++i) {
        const float bottom_x = vanishing_x + static_cast<float>(i) * w * 0.10F;
        SDL_RenderDrawLineF(r, vanishing_x, horizon, bottom_x, h);
    }
}

void draw_palette(SDL_Renderer* r, float w, float h, const WorkbenchState& s) {
    const float pw = palette_width_for(w);
    set_color(r, 11, 16, 18);
    fill_rect(r, {0, 72, pw, h - 72});
    set_color(r, 57, 255, 20);
    draw_text(r, "PARTS", 22, 88, 2.0F);
    draw_text(r, "CLICK OR USE ARROWS", 22, 112, 1.15F);
    const std::array<float, 6> ys{146, 204, 262, 320, 378, 436};
    for (std::size_t i = 0; i < ys.size(); ++i) {
        const bool focused = s.focus == FocusZone::Palette && s.palette_focus == i;
        RectF slot{18, ys[i], pw - 36, 46};
        draw_raised_box(r, slot, focused);
        const PartKind kind = static_cast<PartKind>(i);
        draw_part(r, {kind, part_rect(kind, 47, ys[i] + 23)}, focused);
        set_color(r, 225, 235, 238);
        draw_text(r, info(kind).code, 82, ys[i] + 8, 1.5F);
        set_color(r, focused ? 57 : 160, focused ? 255 : 175, focused ? 20 : 180);
        draw_text(r, info(kind).name, 82, ys[i] + 26, 1.25F);
    }
}

void draw_board(SDL_Renderer* r, float w, float h, const WorkbenchState& s) {
    const RectF board = board_rect_for(w, h);
    set_color(r, 2, 5, 5, 150);
    fill_rect(r, {board.x + 18, board.y + 22, board.w, board.h});
    set_color(r, 8, 37, 29);
    fill_rect(r, {board.x + 8, board.y + 10, board.w, board.h});
    set_color(r, 17, 62, 46);
    fill_rect(r, board);
    set_color(r, 57, 255, 20);
    draw_rect(r, board);
    set_color(r, 34, 91, 70);
    for (float x = board.x + 24; x < board.x + board.w; x += 24) {
        SDL_RenderDrawLineF(r, x, board.y, x, board.y + board.h);
    }
    for (float y = board.y + 24; y < board.y + board.h; y += 24) {
        SDL_RenderDrawLineF(r, board.x, y, board.x + board.w, y);
    }
    set_color(r, 232, 117, 17);
    for (const Wire& wire : s.wires) {
        if (wire.from >= s.parts.size() || wire.to >= s.parts.size()) continue;
        const RectF& a = s.parts[wire.from].rect;
        const RectF& b = s.parts[wire.to].rect;
        SDL_RenderDrawLineF(r, a.x + a.w * 0.5F, a.y + a.h * 0.5F,
                           b.x + b.w * 0.5F, b.y + b.h * 0.5F);
        SDL_RenderDrawLineF(r, a.x + a.w * 0.5F + 1, a.y + a.h * 0.5F + 1,
                           b.x + b.w * 0.5F + 1, b.y + b.h * 0.5F + 1);
    }
    for (std::size_t i = 0; i < s.parts.size(); ++i) {
        const bool selected = (s.has_wire_start && i == s.wire_start) ||
                              (s.focus == FocusZone::BoardPart && i == s.board_focus);
        draw_part(r, s.parts[i], selected);
    }
}

std::string board_part_name(const WorkbenchState& s) {
    if (s.parts.empty() || s.board_focus >= s.parts.size()) return "NONE";
    const PlacedPart& p = s.parts[s.board_focus];
    return std::string(info(p.kind).code) + " " + info(p.kind).name;
}

void draw_inspector(SDL_Renderer* r, float w, float h, const WorkbenchState& s) {
    const RectF panel = inspector_rect_for(w, h);
    set_color(r, 10, 15, 17, 245);
    fill_rect(r, panel);
    set_color(r, 57, 255, 20);
    SDL_RenderDrawLineF(r, panel.x, panel.y, panel.x, h);
    const PartInfo& selected = info(s.selected_kind);
    set_color(r, 57, 255, 20);
    draw_text(r, "PART CODEX", panel.x + 18, panel.y + 18, 1.8F);
    set_color(r, 230, 236, 238);
    draw_text(r, std::string(selected.code) + "  " + selected.name, panel.x + 18, panel.y + 52, 1.55F);
    set_color(r, 170, 188, 192);
    draw_wrapped_text(r, selected.description, panel.x + 18, panel.y + 82, panel.w - 36, 1.25F);

    set_color(r, 57, 255, 20);
    draw_text(r, "PROTOTYPE PAIRS", panel.x + 18, panel.y + 158, 1.45F);
    set_color(r, 205, 214, 216);
    draw_wrapped_text(r, selected.pairs, panel.x + 18, panel.y + 184, panel.w - 36, 1.25F);
    set_color(r, 130, 146, 150);
    draw_wrapped_text(r, "THESE ARE TRAINING SUGGESTIONS. REAL PIN RULES COME LATER.",
                      panel.x + 18, panel.y + 220, panel.w - 36, 1.0F);

    set_color(r, 57, 255, 20);
    draw_text(r, "BOARD FOCUS", panel.x + 18, panel.y + 280, 1.35F);
    set_color(r, 220, 228, 230);
    draw_wrapped_text(r, board_part_name(s), panel.x + 18, panel.y + 304, panel.w - 36, 1.25F);
    if (s.has_wire_start && s.wire_start < s.parts.size()) {
        set_color(r, 255, 211, 80);
        const std::string start = std::string("CONNECT FROM ") + info(s.parts[s.wire_start].kind).code +
                                  " - PICK A COMPATIBLE SECOND PART.";
        draw_wrapped_text(r, start, panel.x + 18, panel.y + 338, panel.w - 36, 1.1F);
    }

    set_color(r, 57, 255, 20);
    draw_text(r, "AVAILABLE TO CONNECT", panel.x + 18, panel.y + 346, 1.2F);
    const std::vector<std::size_t> targets = compatible_targets(s);
    if (targets.empty()) {
        set_color(r, 130, 146, 150);
        draw_text(r, "FOCUS A PLACED PART", panel.x + 18, panel.y + 374, 1.0F);
    } else {
        const std::size_t shown = std::min<std::size_t>(targets.size(), 3);
        for (std::size_t row = 0; row < shown; ++row) {
            const RectF button = target_button_rect(panel, row);
            draw_raised_box(r, button, false);
            set_color(r, 220, 229, 231);
            const PartInfo& target_info = info(s.parts[targets[row]].kind);
            draw_text(r, std::string("CONNECT  ") + target_info.code + "  " + target_info.name,
                      button.x + 10, button.y + 8, 1.0F);
        }
    }

    set_color(r, 57, 255, 20);
    draw_text(r, "NAVIGATION", panel.x + 18, panel.y + 492, 1.35F);
    set_color(r, 196, 207, 210);
    draw_wrapped_text(r,
        "MOUSE CLICK: SELECT OR PLACE. RIGHT CLICK: CONNECT. TAB: MOVE FOCUS. ARROWS: BROWSE. ENTER OR SPACE: USE. V: TEST. C: CLEAR. H: HELP. ESC: EXIT.",
        panel.x + 18, panel.y + 518, panel.w - 36, 1.0F, 4.0F);

    set_color(r, s.validation_ran && s.validation_passed ? 57 : 230,
              s.validation_ran && s.validation_passed ? 255 : 205,
              s.validation_ran && s.validation_passed ? 20 : 100);
    draw_text(r, "STATUS", panel.x + 18, h - 92, 1.35F);
    set_color(r, 225, 232, 234);
    draw_wrapped_text(r, s.status, panel.x + 18, h - 66, panel.w - 36, 1.05F);
}

void draw_top_bar(SDL_Renderer* r, float w, const WorkbenchState& s) {
    set_color(r, 14, 20, 22);
    fill_rect(r, {0, 0, w, 72});
    set_color(r, 57, 255, 20);
    fill_rect(r, {0, 69, w, 3});
    draw_text(r, "FORMFACTOR 3D WORKBENCH", 20, 20, 2.1F);
    set_color(r, 160, 176, 180);
    draw_text(r, "TAB / ARROWS / ENTER OR MOUSE", 20, 48, 1.0F);

    const RectF cb = clear_button_for(w);
    draw_raised_box(r, cb, s.focus == FocusZone::Clear);
    set_color(r, 230, 235, 237);
    draw_text(r, "CLEAR C", cb.x + 16, cb.y + 10, 1.4F);

    const RectF vb = validate_button_for(w);
    draw_raised_box(r, vb, s.focus == FocusZone::Validate);
    set_color(r, s.validation_ran && s.validation_passed ? 57 : 230,
              s.validation_ran && s.validation_passed ? 255 : 235,
              s.validation_ran && s.validation_passed ? 20 : 237);
    draw_text(r, "TEST V", vb.x + 21, vb.y + 10, 1.4F);
}

void draw_help_overlay(SDL_Renderer* r, float w, float h) {
    const RectF q{w * 0.24F, h * 0.20F, w * 0.48F, h * 0.55F};
    set_color(r, 3, 6, 7, 238);
    fill_rect(r, q);
    set_color(r, 57, 255, 20);
    draw_rect(r, q);
    draw_text(r, "HOW TO MOVE AROUND", q.x + 28, q.y + 28, 2.0F);
    set_color(r, 225, 232, 234);
    draw_wrapped_text(r,
        "1. CLICK A PART ON THE LEFT OR USE TAB AND ARROW KEYS. 2. CLICK EMPTY BOARD SPACE TO PLACE IT. 3. RIGHT CLICK TWO PARTS TO CONNECT THEM, OR FOCUS A PLACED PART WITH TAB AND PRESS ENTER ON TWO PARTS. 4. USE TEST TO CHECK THE FIRST POWER RULE. 5. THE RIGHT CODEX EXPLAINS EACH ITEM AND SHOWS ITS SIMPLE PROTOTYPE PAIRS. 6. PRESS H TO HIDE OR SHOW THIS HELP.",
        q.x + 28, q.y + 78, q.w - 56, 1.25F, 6.0F);
    set_color(r, 150, 166, 170);
    draw_wrapped_text(r,
        "THE BOARD USES A DEPTH-STYLED WORKBENCH NOW. THIS IS THE VISUAL AND NAVIGATION FOUNDATION BEFORE A FULL 3D ENGINE PASS.",
        q.x + 28, q.y + q.h - 90, q.w - 56, 1.05F, 4.0F);
}

void draw_workbench(SDL_Renderer* r, int width, int height, const WorkbenchState& s) {
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);
    draw_floor(r, w, h);
    draw_board(r, w, h, s);
    draw_palette(r, w, h, s);
    draw_inspector(r, w, h, s);
    draw_top_bar(r, w, s);
    if (s.show_help) draw_help_overlay(r, w, h);
    SDL_RenderPresent(r);
}

bool pick_palette(WorkbenchState& s, float x, float y, float width) {
    const float pw = palette_width_for(width);
    const std::array<float, 6> ys{146, 204, 262, 320, 378, 436};
    for (std::size_t i = 0; i < ys.size(); ++i) {
        if (contains({18, ys[i], pw - 36, 46}, x, y)) {
            s.selected_kind = static_cast<PartKind>(i);
            s.palette_focus = i;
            s.focus = FocusZone::Palette;
            s.validation_ran = false;
            s.status = std::string("SELECTED ") + info(s.selected_kind).code + " " + info(s.selected_kind).name + ".";
            return true;
        }
    }
    return false;
}

bool find_part(const WorkbenchState& s, float x, float y, std::size_t& index) {
    for (std::size_t n = s.parts.size(); n > 0; --n) {
        const std::size_t candidate = n - 1;
        if (contains(s.parts[candidate].rect, x, y)) {
            index = candidate;
            return true;
        }
    }
    return false;
}

void place_part(WorkbenchState& s, float x, float y, const RectF& board) {
    RectF q = part_rect(s.selected_kind, x, y);
    q.x = std::clamp(q.x, board.x + 8, board.x + board.w - q.w - 8);
    q.y = std::clamp(q.y, board.y + 8, board.y + board.h - q.h - 8);
    s.parts.push_back({s.selected_kind, q});
    s.board_focus = s.parts.size() - 1;
    s.focus = FocusZone::BoardPart;
    s.validation_ran = false;
    s.status = std::string("PLACED ") + info(s.selected_kind).code + ". TAB OR CLICK TO CHOOSE WHAT NEXT.";
}

void connect_part(WorkbenchState& s, std::size_t index) {
    if (index >= s.parts.size()) return;
    if (!s.has_wire_start) {
        s.has_wire_start = true;
        s.wire_start = index;
        s.board_focus = index;
        s.focus = FocusZone::BoardPart;
        s.status = std::string("CONNECT FROM ") + info(s.parts[index].kind).code + ". CHOOSE A SECOND PART.";
        return;
    }
    if (s.wire_start == index) {
        s.has_wire_start = false;
        s.status = "CONNECTION CANCELLED.";
        return;
    }
    const PartKind a = s.parts[s.wire_start].kind;
    const PartKind b = s.parts[index].kind;
    if (!prototype_pair_allowed(a, b)) {
        s.status = std::string("PAIR BLOCKED IN PROTOTYPE: ") + info(a).code + " + " + info(b).code + ". CHECK CODEX.";
        s.has_wire_start = false;
        return;
    }
    bool duplicate = false;
    for (const Wire& wire : s.wires) {
        if ((wire.from == s.wire_start && wire.to == index) ||
            (wire.from == index && wire.to == s.wire_start)) {
            duplicate = true;
            break;
        }
    }
    if (!duplicate) {
        s.wires.push_back({s.wire_start, index});
        s.validation_ran = false;
        s.status = std::string("CONNECTED ") + info(a).code + " + " + info(b).code + ".";
    } else {
        s.status = "THOSE PARTS ARE ALREADY CONNECTED.";
    }
    s.has_wire_start = false;
}

bool click_compatible_target(WorkbenchState& s, float x, float y, float width, float height) {
    const RectF panel = inspector_rect_for(width, height);
    const std::vector<std::size_t> targets = compatible_targets(s);
    const std::size_t shown = std::min<std::size_t>(targets.size(), 3);
    for (std::size_t row = 0; row < shown; ++row) {
        if (!contains(target_button_rect(panel, row), x, y)) continue;
        if (!s.has_wire_start && s.focus == FocusZone::BoardPart && s.board_focus < s.parts.size()) {
            connect_part(s, s.board_focus);
        }
        if (s.has_wire_start) connect_part(s, targets[row]);
        return true;
    }
    return false;
}

void clear_board(WorkbenchState& s) {
    const PartKind keep = s.selected_kind;
    const std::size_t keep_focus = s.palette_focus;
    s = WorkbenchState{};
    s.selected_kind = keep;
    s.palette_focus = keep_focus;
    s.status = "BOARD CLEARED - CHOOSE A PART.";
}

void cycle_focus(WorkbenchState& s, int direction) {
    std::array<FocusZone, 4> zones{FocusZone::Palette, FocusZone::BoardPart, FocusZone::Validate, FocusZone::Clear};
    int current = 0;
    for (int i = 0; i < static_cast<int>(zones.size()); ++i) {
        if (zones[static_cast<std::size_t>(i)] == s.focus) current = i;
    }
    for (int tries = 0; tries < 4; ++tries) {
        current = (current + direction + 4) % 4;
        const FocusZone candidate = zones[static_cast<std::size_t>(current)];
        if (candidate != FocusZone::BoardPart || !s.parts.empty()) {
            s.focus = candidate;
            break;
        }
    }
}

void browse_focus(WorkbenchState& s, int direction) {
    if (s.focus == FocusZone::Palette) {
        const int size = static_cast<int>(kPartInfo.size());
        int index = static_cast<int>(s.palette_focus);
        index = (index + direction + size) % size;
        s.palette_focus = static_cast<std::size_t>(index);
        s.selected_kind = static_cast<PartKind>(s.palette_focus);
        s.status = std::string("FOCUS ") + info(s.selected_kind).code + " " + info(s.selected_kind).name + ". ENTER TO SELECT.";
    } else if (s.focus == FocusZone::BoardPart && !s.parts.empty()) {
        const int size = static_cast<int>(s.parts.size());
        int index = static_cast<int>(std::min(s.board_focus, s.parts.size() - 1));
        index = (index + direction + size) % size;
        s.board_focus = static_cast<std::size_t>(index);
        s.status = std::string("BOARD FOCUS ") + info(s.parts[s.board_focus].kind).code + ". ENTER TO START OR FINISH A CONNECTION.";
    } else if (s.focus == FocusZone::Validate || s.focus == FocusZone::Clear) {
        s.focus = s.focus == FocusZone::Validate ? FocusZone::Clear : FocusZone::Validate;
    }
}

void activate_focus(WorkbenchState& s) {
    switch (s.focus) {
        case FocusZone::Palette:
            s.selected_kind = static_cast<PartKind>(s.palette_focus);
            s.status = std::string("SELECTED ") + info(s.selected_kind).code + " " + info(s.selected_kind).name + ". CLICK BOARD TO PLACE.";
            break;
        case FocusZone::BoardPart:
            if (!s.parts.empty()) connect_part(s, std::min(s.board_focus, s.parts.size() - 1));
            break;
        case FocusZone::Validate:
            run_validation(s);
            break;
        case FocusZone::Clear:
            clear_board(s);
            break;
    }
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
        "FormFactor 3D Workbench prototype", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 1440, 860,
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

    WorkbenchState state;
    update_window_title(window, state);
    bool running = true;
    while (running) {
        SDL_Event event{};
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                const SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE) running = false;
                else if (key == SDLK_h) state.show_help = !state.show_help;
                else if (key == SDLK_c) clear_board(state);
                else if (key == SDLK_v) run_validation(state);
                else if (key == SDLK_TAB) cycle_focus(state, (event.key.keysym.mod & KMOD_SHIFT) ? -1 : 1);
                else if (key == SDLK_UP || key == SDLK_LEFT) browse_focus(state, -1);
                else if (key == SDLK_DOWN || key == SDLK_RIGHT) browse_focus(state, 1);
                else if (key == SDLK_RETURN || key == SDLK_SPACE) activate_focus(state);
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
                        state.focus = FocusZone::Validate;
                        run_validation(state);
                    } else if (contains(clear_button_for(w), x, y)) {
                        state.focus = FocusZone::Clear;
                        clear_board(state);
                    } else if (click_compatible_target(state, x, y, w, h)) {
                        // The inspector target list starts or completes the connection.
                    } else if (!pick_palette(state, x, y, w)) {
                        std::size_t index = 0;
                        if (find_part(state, x, y, index)) {
                            state.focus = FocusZone::BoardPart;
                            state.board_focus = index;
                            state.status = std::string("BOARD FOCUS ") + info(state.parts[index].kind).code + ". RIGHT CLICK OR ENTER TO CONNECT.";
                        } else if (contains(board, x, y)) {
                            place_part(state, x, y, board);
                        }
                    }
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    std::size_t index = 0;
                    if (find_part(state, x, y, index)) connect_part(state, index);
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
