#include <SDL2/SDL.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace {

struct RectF { float x; float y; float w; float h; };
enum class PartKind : std::size_t { Resistor = 0, Capacitor, Led, Chip, Power, Connector };
enum class FocusZone { Palette, BoardPart, Validate, Clear, Help };
struct PlacedPart { PartKind kind; RectF rect; };
struct Wire { std::size_t from; std::size_t to; };

struct PartInfo {
    const char* code;
    const char* reference;
    const char* name;
    const char* package;
    const char* description;
    const char* pairs;
};

constexpr std::array<PartInfo, 6> kPartInfo{{
    {"R", "R", "RESISTOR", "AXIAL THROUGH-HOLE", "LIMITS CURRENT AND CREATES A CONTROLLED VOLTAGE DROP.", "PWR, LED, IC, J, C"},
    {"C", "C", "CAPACITOR", "RADIAL ELECTROLYTIC", "STORES CHARGE AND HELPS SMOOTH OR FILTER VOLTAGE CHANGES.", "PWR, R, IC, J"},
    {"LED", "D", "LED", "5 MM THROUGH-HOLE LED", "LIGHTS WHEN CURRENT FLOWS THROUGH IT IN THE CORRECT DIRECTION.", "PWR, R, IC, J"},
    {"IC", "U", "CHIP", "DIP INTEGRATED CIRCUIT", "REPRESENTS AN INTEGRATED CIRCUIT THAT USES POWER AND SIGNALS.", "PWR, R, C, LED, J"},
    {"PWR", "BT", "POWER", "BATTERY STYLE SOURCE", "SUPPLIES ELECTRICAL ENERGY TO THE TRAINING BOARD.", "R, C, LED, IC, J"},
    {"J", "J", "CONNECTOR", "1 X 6 PIN HEADER", "GIVES POWER OR SIGNALS A PHYSICAL PATH INTO OR OUT OF THE BOARD.", "PWR, R, C, LED, IC"},
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

void fill_circle(SDL_Renderer* r, float cx, float cy, float radius) {
    const int rr = static_cast<int>(radius);
    for (int dy = -rr; dy <= rr; ++dy) {
        const float inside = radius * radius - static_cast<float>(dy * dy);
        if (inside < 0.0F) continue;
        const float dx = std::sqrt(inside);
        SDL_RenderDrawLineF(r, cx - dx, cy + static_cast<float>(dy),
                           cx + dx, cy + static_cast<float>(dy));
    }
}

void draw_circle(SDL_Renderer* r, float cx, float cy, float radius) {
    const int steps = 40;
    float px = cx + radius;
    float py = cy;
    constexpr float pi = 3.14159265358979323846F;
    for (int i = 1; i <= steps; ++i) {
        const float angle = 2.0F * pi * static_cast<float>(i) / static_cast<float>(steps);
        const float x = cx + std::cos(angle) * radius;
        const float y = cy + std::sin(angle) * radius;
        SDL_RenderDrawLineF(r, px, py, x, y);
        px = x;
        py = y;
    }
}

RectF part_rect(PartKind kind, float x, float y) {
    switch (kind) {
        case PartKind::Resistor: return {x - 43, y - 16, 86, 32};
        case PartKind::Capacitor: return {x - 26, y - 31, 52, 62};
        case PartKind::Led: return {x - 25, y - 29, 50, 58};
        case PartKind::Chip: return {x - 50, y - 34, 100, 68};
        case PartKind::Power: return {x - 44, y - 32, 88, 64};
        case PartKind::Connector: return {x - 30, y - 43, 60, 86};
    }
    return {x - 25, y - 20, 50, 40};
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

void draw_resistor_physical(SDL_Renderer* r, const RectF& q, bool selected) {
    const float cy = q.y + q.h * 0.5F;
    set_color(r, 190, 196, 198);
    SDL_RenderDrawLineF(r, q.x, cy, q.x + q.w * 0.20F, cy);
    SDL_RenderDrawLineF(r, q.x + q.w * 0.80F, cy, q.x + q.w, cy);
    const RectF body{q.x + q.w * 0.18F, q.y + q.h * 0.18F, q.w * 0.64F, q.h * 0.64F};
    set_color(r, 210, 174, 108);
    fill_rect(r, body);
    set_color(r, selected ? 255 : 78, selected ? 225 : 58, selected ? 70 : 42);
    draw_rect(r, body);
    const std::array<std::array<std::uint8_t, 3>, 4> bands{{
        {{92, 52, 30}}, {{180, 35, 35}}, {{32, 32, 32}}, {{214, 168, 38}}
    }};
    for (std::size_t i = 0; i < bands.size(); ++i) {
        const float bx = body.x + 8.0F + static_cast<float>(i) * (body.w - 18.0F) / 4.0F;
        set_color(r, bands[i][0], bands[i][1], bands[i][2]);
        fill_rect(r, {bx, body.y + 1.0F, 4.0F, body.h - 2.0F});
    }
    set_color(r, 255, 245, 210, 120);
    SDL_RenderDrawLineF(r, body.x + 3, body.y + 3, body.x + body.w - 3, body.y + 3);
}

void draw_capacitor_physical(SDL_Renderer* r, const RectF& q, bool selected) {
    const float cx = q.x + q.w * 0.5F;
    set_color(r, 190, 196, 198);
    SDL_RenderDrawLineF(r, cx - 8, q.y + q.h * 0.78F, cx - 8, q.y + q.h);
    SDL_RenderDrawLineF(r, cx + 8, q.y + q.h * 0.78F, cx + 8, q.y + q.h);
    const RectF can{q.x + 7, q.y + 4, q.w - 14, q.h * 0.76F};
    set_color(r, 38, 91, 168);
    fill_rect(r, can);
    set_color(r, selected ? 255 : 205, selected ? 225 : 214, selected ? 70 : 224);
    draw_rect(r, can);
    set_color(r, 220, 224, 228);
    fill_rect(r, {can.x, can.y, can.w, 6});
    set_color(r, 238, 238, 238);
    fill_rect(r, {can.x + can.w - 7, can.y + 6, 5, can.h - 9});
    set_color(r, 20, 24, 30);
    draw_text(r, "+", can.x + 5, can.y + 10, 1.2F);
}

void draw_led_physical(SDL_Renderer* r, const RectF& q, bool selected) {
    const float cx = q.x + q.w * 0.5F;
    const float head_y = q.y + q.h * 0.36F;
    set_color(r, 184, 190, 192);
    SDL_RenderDrawLineF(r, cx - 7, q.y + q.h * 0.58F, cx - 7, q.y + q.h);
    SDL_RenderDrawLineF(r, cx + 7, q.y + q.h * 0.58F, cx + 7, q.y + q.h - 5);
    set_color(r, 194, 28, 38);
    fill_circle(r, cx, head_y, q.w * 0.28F);
    set_color(r, selected ? 255 : 252, selected ? 225 : 94, selected ? 70 : 105);
    draw_circle(r, cx, head_y, q.w * 0.28F);
    set_color(r, 255, 255, 255, 130);
    fill_circle(r, cx - 5, head_y - 6, 3.0F);
}

void draw_chip_physical(SDL_Renderer* r, const RectF& q, bool selected) {
    const RectF body{q.x + 12, q.y + 7, q.w - 24, q.h - 14};
    set_color(r, 24, 27, 30);
    fill_rect(r, body);
    set_color(r, selected ? 255 : 177, selected ? 225 : 183, selected ? 70 : 187);
    draw_rect(r, body);
    set_color(r, 198, 202, 204);
    for (int i = 0; i < 6; ++i) {
        const float py = body.y + 6.0F + static_cast<float>(i) * (body.h - 12.0F) / 5.0F;
        fill_rect(r, {q.x + 2, py - 2, 10, 4});
        fill_rect(r, {q.x + q.w - 12, py - 2, 10, 4});
    }
    set_color(r, 120, 126, 130);
    draw_circle(r, body.x + body.w * 0.5F, body.y + 5, 5.0F);
    set_color(r, 223, 226, 228);
    draw_text(r, "IC", body.x + body.w * 0.5F - 10, body.y + body.h * 0.5F - 5, 1.4F);
}

void draw_power_physical(SDL_Renderer* r, const RectF& q, bool selected) {
    const RectF body{q.x + 5, q.y + 10, q.w - 10, q.h - 15};
    set_color(r, 44, 47, 52);
    fill_rect(r, body);
    set_color(r, selected ? 255 : 132, selected ? 225 : 138, selected ? 70 : 143);
    draw_rect(r, body);
    set_color(r, 192, 44, 48);
    fill_rect(r, {body.x + 8, q.y + 3, 18, 12});
    set_color(r, 54, 57, 61);
    fill_rect(r, {body.x + body.w - 26, q.y + 3, 18, 12});
    set_color(r, 235, 238, 240);
    draw_text(r, "+", body.x + 11, body.y + 17, 1.7F);
    draw_text(r, "-", body.x + body.w - 22, body.y + 17, 1.7F);
    set_color(r, 57, 255, 20);
    draw_text(r, "PWR", body.x + 20, body.y + body.h - 18, 1.4F);
}

void draw_connector_physical(SDL_Renderer* r, const RectF& q, bool selected) {
    const RectF body{q.x + 8, q.y + 4, q.w - 16, q.h - 8};
    set_color(r, 26, 29, 31);
    fill_rect(r, body);
    set_color(r, selected ? 255 : 105, selected ? 225 : 111, selected ? 70 : 115);
    draw_rect(r, body);
    for (int i = 0; i < 6; ++i) {
        const float py = body.y + 7.0F + static_cast<float>(i) * (body.h - 14.0F) / 5.0F;
        set_color(r, 218, 170, 48);
        fill_rect(r, {body.x + body.w * 0.30F - 3, py - 3, 6, 6});
        fill_rect(r, {body.x + body.w * 0.70F - 3, py - 3, 6, 6});
    }
}

void draw_physical_part(SDL_Renderer* r, const PlacedPart& p, bool selected) {
    set_color(r, 2, 4, 5, 185);
    fill_rect(r, {p.rect.x + 6, p.rect.y + 7, p.rect.w, p.rect.h});
    switch (p.kind) {
        case PartKind::Resistor: draw_resistor_physical(r, p.rect, selected); break;
        case PartKind::Capacitor: draw_capacitor_physical(r, p.rect, selected); break;
        case PartKind::Led: draw_led_physical(r, p.rect, selected); break;
        case PartKind::Chip: draw_chip_physical(r, p.rect, selected); break;
        case PartKind::Power: draw_power_physical(r, p.rect, selected); break;
        case PartKind::Connector: draw_connector_physical(r, p.rect, selected); break;
    }
}

void draw_schematic_symbol(SDL_Renderer* r, PartKind kind, const RectF& area,
                           bool highlighted = false) {
    const float cx = area.x + area.w * 0.5F;
    const float cy = area.y + area.h * 0.5F;
    const float left = area.x + 4.0F;
    const float right = area.x + area.w - 4.0F;
    set_color(r, highlighted ? 255 : 225, highlighted ? 225 : 232,
              highlighted ? 70 : 235);

    if (kind == PartKind::Resistor) {
        const float x0 = left + 4.0F;
        const float x1 = right - 4.0F;
        SDL_RenderDrawLineF(r, left, cy, x0, cy);
        const int zig = 8;
        float px = x0;
        float py = cy;
        for (int i = 1; i <= zig; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(zig);
            const float nx = x0 + (x1 - x0) * t;
            float ny = cy;
            if (i < zig) ny += (i % 2 == 0 ? -1.0F : 1.0F) * area.h * 0.22F;
            SDL_RenderDrawLineF(r, px, py, nx, ny);
            px = nx;
            py = ny;
        }
        SDL_RenderDrawLineF(r, x1, cy, right, cy);
    } else if (kind == PartKind::Capacitor) {
        const float plate1 = cx - 5.0F;
        const float plate2 = cx + 5.0F;
        SDL_RenderDrawLineF(r, left, cy, plate1, cy);
        SDL_RenderDrawLineF(r, plate2, cy, right, cy);
        SDL_RenderDrawLineF(r, plate1, cy - area.h * 0.28F, plate1, cy + area.h * 0.28F);
        SDL_RenderDrawLineF(r, plate2, cy - area.h * 0.28F, plate2, cy + area.h * 0.28F);
        draw_text(r, "+", plate1 - 11, cy - area.h * 0.40F, 0.8F);
    } else if (kind == PartKind::Led) {
        const float diode_left = cx - 10.0F;
        const float diode_right = cx + 7.0F;
        SDL_RenderDrawLineF(r, left, cy, diode_left, cy);
        SDL_RenderDrawLineF(r, diode_left, cy - 10, diode_right, cy);
        SDL_RenderDrawLineF(r, diode_left, cy + 10, diode_right, cy);
        SDL_RenderDrawLineF(r, diode_left, cy - 10, diode_left, cy + 10);
        SDL_RenderDrawLineF(r, diode_right + 3, cy - 11, diode_right + 3, cy + 11);
        SDL_RenderDrawLineF(r, diode_right + 3, cy, right, cy);
        SDL_RenderDrawLineF(r, cx + 4, cy - 13, cx + 14, cy - 23);
        SDL_RenderDrawLineF(r, cx + 14, cy - 23, cx + 10, cy - 20);
        SDL_RenderDrawLineF(r, cx + 14, cy - 23, cx + 11, cy - 16);
        SDL_RenderDrawLineF(r, cx + 10, cy - 8, cx + 20, cy - 18);
    } else if (kind == PartKind::Chip) {
        const RectF body{cx - area.w * 0.23F, cy - area.h * 0.30F,
                         area.w * 0.46F, area.h * 0.60F};
        draw_rect(r, body);
        for (int i = 0; i < 3; ++i) {
            const float py = body.y + body.h * (static_cast<float>(i + 1) / 4.0F);
            SDL_RenderDrawLineF(r, left, py, body.x, py);
            SDL_RenderDrawLineF(r, body.x + body.w, py, right, py);
        }
        draw_text(r, "U", cx - 4, cy - 5, 1.1F);
    } else if (kind == PartKind::Power) {
        const float plate1 = cx - 5.0F;
        const float plate2 = cx + 6.0F;
        SDL_RenderDrawLineF(r, left, cy, plate1, cy);
        SDL_RenderDrawLineF(r, plate2, cy, right, cy);
        SDL_RenderDrawLineF(r, plate1, cy - area.h * 0.34F, plate1, cy + area.h * 0.34F);
        SDL_RenderDrawLineF(r, plate2, cy - area.h * 0.19F, plate2, cy + area.h * 0.19F);
        draw_text(r, "+", plate1 - 12, cy - area.h * 0.45F, 0.8F);
        draw_text(r, "-", plate2 + 5, cy - area.h * 0.34F, 0.8F);
    } else if (kind == PartKind::Connector) {
        for (int i = 0; i < 4; ++i) {
            const float py = area.y + 8.0F + static_cast<float>(i) * (area.h - 16.0F) / 3.0F;
            SDL_RenderDrawLineF(r, left, py, cx - 5, py);
            draw_circle(r, cx, py, 4.0F);
            SDL_RenderDrawLineF(r, cx + 4, py, right, py);
        }
    }
}

std::string reference_for(const std::vector<PlacedPart>& parts, std::size_t index) {
    if (index >= parts.size()) return "";
    const PartKind kind = parts[index].kind;
    std::size_t count = 0;
    for (std::size_t i = 0; i <= index; ++i) {
        if (parts[i].kind == kind) ++count;
    }
    return std::string(info(kind).reference) + std::to_string(count);
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

float palette_width_for(float w) { return std::min(320.0F, w * 0.23F); }
float inspector_width_for(float w) { return std::min(390.0F, w * 0.29F); }
RectF board_rect_for(float w, float h) {
    const float left = palette_width_for(w);
    const float right = inspector_width_for(w);
    const float margin = 26.0F;
    return {left + margin, 104.0F,
            std::max(360.0F, w - left - right - margin * 2.0F),
            std::max(280.0F, h - 104.0F - 54.0F)};
}
RectF inspector_rect_for(float w, float h) {
    const float iw = inspector_width_for(w);
    return {w - iw, 72.0F, iw, h - 72.0F};
}
RectF validate_button_for(float w) { return {w - inspector_width_for(w) - 148.0F, 18.0F, 128.0F, 34.0F}; }
RectF clear_button_for(float w) { return {w - inspector_width_for(w) - 284.0F, 18.0F, 120.0F, 34.0F}; }
RectF help_button_for(float w) { return {w - inspector_width_for(w) - 410.0F, 18.0F, 110.0F, 34.0F}; }

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
    return {panel.x + 18.0F, panel.y + 530.0F + static_cast<float>(row) * 34.0F,
            panel.w - 36.0F, 27.0F};
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
    const float vanishing_x = w * 0.53F;
    for (int i = -8; i <= 8; ++i) {
        const float bottom_x = vanishing_x + static_cast<float>(i) * w * 0.10F;
        SDL_RenderDrawLineF(r, vanishing_x, horizon, bottom_x, h);
    }
}

void draw_symbol_badge(SDL_Renderer* r, PartKind kind, const RectF& physical,
                       const RectF& board, const std::string& reference,
                       bool highlighted) {
    RectF badge{physical.x + physical.w + 7.0F, physical.y - 3.0F, 68.0F, 43.0F};
    if (badge.x + badge.w > board.x + board.w - 4.0F) badge.x = physical.x - badge.w - 7.0F;
    if (badge.y < board.y + 4.0F) badge.y = physical.y + physical.h + 4.0F;
    set_color(r, 7, 11, 12, 235);
    fill_rect(r, badge);
    set_color(r, highlighted ? 255 : 57, highlighted ? 225 : 255,
              highlighted ? 70 : 20);
    draw_rect(r, badge);
    draw_schematic_symbol(r, kind,
                          {badge.x + 3, badge.y + 10, badge.w - 6, badge.h - 13},
                          highlighted);
    set_color(r, 210, 222, 224);
    draw_text(r, reference, badge.x + 4, badge.y + 3, 0.8F);
}

void draw_palette(SDL_Renderer* r, float w, float h, const WorkbenchState& s) {
    const float pw = palette_width_for(w);
    set_color(r, 11, 16, 18);
    fill_rect(r, {0, 72, pw, h - 72});
    set_color(r, 57, 255, 20);
    draw_text(r, "PHYSICAL PARTS + SYMBOLS", 18, 88, 1.5F);
    set_color(r, 145, 161, 165);
    draw_text(r, "CLICK OR USE ARROWS", 18, 112, 1.05F);
    const std::array<float, 6> ys{142, 210, 278, 346, 414, 482};
    for (std::size_t i = 0; i < ys.size(); ++i) {
        const bool focused = s.focus == FocusZone::Palette && s.palette_focus == i;
        const RectF slot{14, ys[i], pw - 28, 58};
        draw_raised_box(r, slot, focused);
        const PartKind kind = static_cast<PartKind>(i);
        const RectF mini = part_rect(kind, slot.x + 49, slot.y + slot.h * 0.5F);
        const float scale_x = 0.68F;
        const float scale_y = 0.68F;
        RectF small{mini.x + (mini.w * (1.0F - scale_x)) * 0.5F,
                    mini.y + (mini.h * (1.0F - scale_y)) * 0.5F,
                    mini.w * scale_x, mini.h * scale_y};
        draw_physical_part(r, {kind, small}, focused);
        const RectF symbol_area{slot.x + 88, slot.y + 10, 70, 38};
        set_color(r, 8, 12, 14);
        fill_rect(r, symbol_area);
        set_color(r, focused ? 255 : 96, focused ? 225 : 114, focused ? 70 : 118);
        draw_rect(r, symbol_area);
        draw_schematic_symbol(r, kind, {symbol_area.x + 3, symbol_area.y + 3,
                                        symbol_area.w - 6, symbol_area.h - 6}, focused);
        set_color(r, 225, 235, 238);
        draw_text(r, info(kind).code, slot.x + 168, slot.y + 8, 1.35F);
        set_color(r, focused ? 57 : 165, focused ? 255 : 181, focused ? 20 : 186);
        draw_text(r, info(kind).name, slot.x + 168, slot.y + 27, 1.05F);
        set_color(r, 120, 136, 140);
        draw_text(r, info(kind).reference, slot.x + 168, slot.y + 43, 0.85F);
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
    set_color(r, 210, 226, 220);
    draw_text(r, "PHYSICAL BOARD", board.x + 12, board.y + 10, 1.05F);
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
        draw_physical_part(r, s.parts[i], selected);
        const std::string reference = reference_for(s.parts, i);
        set_color(r, selected ? 255 : 225, selected ? 225 : 232, selected ? 70 : 235);
        draw_text(r, reference, s.parts[i].rect.x + 2, s.parts[i].rect.y - 13, 0.9F);
        draw_symbol_badge(r, s.parts[i].kind, s.parts[i].rect, board, reference, selected);
    }
}

std::string board_part_name(const WorkbenchState& s) {
    if (s.parts.empty() || s.board_focus >= s.parts.size()) return "NONE";
    const PlacedPart& p = s.parts[s.board_focus];
    return reference_for(s.parts, s.board_focus) + " " + info(p.kind).name;
}

void draw_schematic_mirror(SDL_Renderer* r, const RectF& panel, float w, float h,
                           const WorkbenchState& s) {
    const RectF box{panel.x + 16, panel.y + 45, panel.w - 32, 220};
    set_color(r, 4, 9, 11);
    fill_rect(r, box);
    set_color(r, 73, 96, 100);
    draw_rect(r, box);
    set_color(r, 28, 43, 46);
    for (float x = box.x + 24; x < box.x + box.w; x += 24) SDL_RenderDrawLineF(r, x, box.y, x, box.y + box.h);
    for (float y = box.y + 24; y < box.y + box.h; y += 24) SDL_RenderDrawLineF(r, box.x, y, box.x + box.w, y);

    if (s.parts.empty()) {
        set_color(r, 127, 145, 149);
        draw_wrapped_text(r, "PLACE A PHYSICAL PART AND ITS SCHEMATIC SYMBOL WILL APPEAR HERE.",
                          box.x + 18, box.y + 82, box.w - 36, 1.0F, 4.0F);
        return;
    }

    const RectF board = board_rect_for(w, h);
    auto symbol_center = [&](std::size_t index) {
        const RectF& p = s.parts[index].rect;
        const float px = p.x + p.w * 0.5F;
        const float py = p.y + p.h * 0.5F;
        const float nx = std::clamp((px - board.x) / std::max(1.0F, board.w), 0.0F, 1.0F);
        const float ny = std::clamp((py - board.y) / std::max(1.0F, board.h), 0.0F, 1.0F);
        return std::array<float, 2>{box.x + 35 + nx * (box.w - 70),
                                    box.y + 30 + ny * (box.h - 60)};
    };

    set_color(r, 232, 117, 17);
    for (const Wire& wire : s.wires) {
        if (wire.from >= s.parts.size() || wire.to >= s.parts.size()) continue;
        const auto a = symbol_center(wire.from);
        const auto b = symbol_center(wire.to);
        SDL_RenderDrawLineF(r, a[0], a[1], b[0], b[1]);
    }

    for (std::size_t i = 0; i < s.parts.size(); ++i) {
        const auto center = symbol_center(i);
        const bool focused = (s.focus == FocusZone::BoardPart && s.board_focus == i) ||
                             (s.has_wire_start && s.wire_start == i);
        const RectF node{center[0] - 28, center[1] - 18, 56, 36};
        set_color(r, 8, 13, 15);
        fill_rect(r, node);
        set_color(r, focused ? 255 : 122, focused ? 225 : 139, focused ? 70 : 143);
        draw_rect(r, node);
        draw_schematic_symbol(r, s.parts[i].kind,
                              {node.x + 3, node.y + 7, node.w - 6, node.h - 10}, focused);
        set_color(r, 222, 230, 232);
        draw_text(r, reference_for(s.parts, i), node.x + 2, node.y + 1, 0.65F);
    }
}

void draw_inspector(SDL_Renderer* r, float w, float h, const WorkbenchState& s) {
    const RectF panel = inspector_rect_for(w, h);
    set_color(r, 10, 15, 17, 248);
    fill_rect(r, panel);
    set_color(r, 57, 255, 20);
    SDL_RenderDrawLineF(r, panel.x, panel.y, panel.x, h);
    draw_text(r, "LIVE SCHEMATIC MIRROR", panel.x + 18, panel.y + 16, 1.45F);
    draw_schematic_mirror(r, panel, w, h, s);
    set_color(r, 118, 135, 139);
    draw_text(r, "MIRRORS BOARD POSITION FOR LEARNING", panel.x + 18, panel.y + 272, 0.85F);

    const PartInfo& selected = info(s.selected_kind);
    set_color(r, 57, 255, 20);
    draw_text(r, "PART CODEX", panel.x + 18, panel.y + 300, 1.45F);
    set_color(r, 230, 236, 238);
    draw_text(r, std::string(selected.code) + "  " + selected.name, panel.x + 18, panel.y + 326, 1.25F);
    set_color(r, 145, 163, 167);
    draw_text(r, selected.package, panel.x + 18, panel.y + 347, 0.85F);
    set_color(r, 170, 188, 192);
    draw_wrapped_text(r, selected.description, panel.x + 18, panel.y + 368, panel.w - 36, 1.0F, 4.0F);

    set_color(r, 57, 255, 20);
    draw_text(r, "CONNECTS WITH", panel.x + 18, panel.y + 426, 1.15F);
    set_color(r, 205, 214, 216);
    draw_wrapped_text(r, selected.pairs, panel.x + 18, panel.y + 447, panel.w - 36, 1.0F);
    set_color(r, 114, 132, 136);
    draw_wrapped_text(r, "TRAINING PAIRS ONLY. REAL PIN RULES MUST COME FROM VERIFIED PART DATA.",
                      panel.x + 18, panel.y + 470, panel.w - 36, 0.82F, 3.0F);

    set_color(r, 57, 255, 20);
    draw_text(r, "AVAILABLE TO CONNECT", panel.x + 18, panel.y + 507, 1.0F);
    const std::vector<std::size_t> targets = compatible_targets(s);
    if (targets.empty()) {
        set_color(r, 130, 146, 150);
        draw_text(r, "FOCUS A PLACED PART", panel.x + 18, panel.y + 535, 0.9F);
    } else {
        const std::size_t shown = std::min<std::size_t>(targets.size(), 3);
        for (std::size_t row = 0; row < shown; ++row) {
            const RectF button = target_button_rect(panel, row);
            draw_raised_box(r, button, false);
            set_color(r, 220, 229, 231);
            const std::size_t target = targets[row];
            draw_text(r, std::string("CONNECT  ") + reference_for(s.parts, target) + "  " + info(s.parts[target].kind).name,
                      button.x + 8, button.y + 8, 0.85F);
        }
    }

    set_color(r, 57, 255, 20);
    draw_text(r, "BOARD FOCUS", panel.x + 18, h - 142, 1.0F);
    set_color(r, 218, 227, 229);
    draw_text(r, board_part_name(s), panel.x + 18, h - 121, 0.9F);
    set_color(r, s.validation_ran && s.validation_passed ? 57 : 230,
              s.validation_ran && s.validation_passed ? 255 : 205,
              s.validation_ran && s.validation_passed ? 20 : 100);
    draw_text(r, "STATUS", panel.x + 18, h - 92, 1.0F);
    set_color(r, 225, 232, 234);
    draw_wrapped_text(r, s.status, panel.x + 18, h - 70, panel.w - 36, 0.9F, 3.0F);
}

void draw_top_bar(SDL_Renderer* r, float w, const WorkbenchState& s) {
    set_color(r, 14, 20, 22);
    fill_rect(r, {0, 0, w, 72});
    set_color(r, 57, 255, 20);
    fill_rect(r, {0, 69, w, 3});
    draw_text(r, "FORMFACTOR WORKBENCH", 20, 17, 2.0F);
    set_color(r, 155, 171, 175);
    draw_text(r, "PHYSICAL PART + SYMBOL + LIVE SCHEMATIC", 20, 45, 0.95F);

    const RectF hb = help_button_for(w);
    draw_raised_box(r, hb, s.focus == FocusZone::Help);
    set_color(r, 230, 235, 237);
    draw_text(r, "HELP H", hb.x + 15, hb.y + 10, 1.2F);

    const RectF cb = clear_button_for(w);
    draw_raised_box(r, cb, s.focus == FocusZone::Clear);
    set_color(r, 230, 235, 237);
    draw_text(r, "CLEAR C", cb.x + 13, cb.y + 10, 1.2F);

    const RectF vb = validate_button_for(w);
    draw_raised_box(r, vb, s.focus == FocusZone::Validate);
    set_color(r, s.validation_ran && s.validation_passed ? 57 : 230,
              s.validation_ran && s.validation_passed ? 255 : 235,
              s.validation_ran && s.validation_passed ? 20 : 237);
    draw_text(r, "TEST V", vb.x + 18, vb.y + 10, 1.2F);
}

void draw_help_overlay(SDL_Renderer* r, float w, float h) {
    const RectF q{w * 0.13F, h * 0.10F, w * 0.74F, h * 0.78F};
    set_color(r, 3, 6, 7, 246);
    fill_rect(r, q);
    set_color(r, 57, 255, 20);
    draw_rect(r, q);
    draw_text(r, "FORMFACTOR CONTROLS", q.x + 28, q.y + 24, 2.0F);
    set_color(r, 150, 169, 173);
    draw_text(r, "PRESS H OR F1 TO CLOSE THIS PAGE", q.x + 28, q.y + 53, 1.0F);

    const float left = q.x + 30;
    const float right = q.x + q.w * 0.52F;
    const float column_w = q.w * 0.42F;
    set_color(r, 57, 255, 20);
    draw_text(r, "QUICK START", left, q.y + 92, 1.45F);
    set_color(r, 224, 232, 234);
    draw_wrapped_text(r,
        "1. CHOOSE A PART ON THE LEFT. 2. CLICK EMPTY GREEN BOARD SPACE TO PLACE THE PHYSICAL PART. 3. EVERY PLACED PART SHOWS ITS SCHEMATIC SYMBOL NEXT TO IT. 4. RIGHT CLICK ONE PART THEN ANOTHER TO CONNECT THEM. 5. CLICK TEST OR PRESS V TO CHECK THE CURRENT PROTOTYPE RULE. 6. USE THE LIVE SCHEMATIC ON THE RIGHT TO SEE THE SAME BUILD AS SYMBOLS.",
        left, q.y + 120, column_w, 1.0F, 5.0F);

    set_color(r, 57, 255, 20);
    draw_text(r, "MOUSE", left, q.y + 350, 1.35F);
    set_color(r, 210, 221, 223);
    draw_wrapped_text(r,
        "LEFT CLICK A PART: SELECT. LEFT CLICK BOARD: PLACE. LEFT CLICK A PLACED PART: FOCUS. RIGHT CLICK TWO PLACED PARTS: CONNECT. CLICK A CONNECT OPTION ON THE RIGHT: CONNECT TO THAT PART.",
        left, q.y + 378, column_w, 1.0F, 5.0F);

    set_color(r, 57, 255, 20);
    draw_text(r, "KEYBOARD", right, q.y + 92, 1.45F);
    set_color(r, 224, 232, 234);
    draw_wrapped_text(r,
        "TAB: MOVE TO THE NEXT CLICKABLE GROUP. SHIFT + TAB: MOVE BACK. ARROW KEYS: MOVE THROUGH PART LABELS OR PLACED PARTS. ENTER OR SPACE: USE THE FOCUSED ITEM. V: TEST. C: CLEAR. H OR F1: HELP PAGE. ESC: EXIT.",
        right, q.y + 120, column_w, 1.0F, 5.0F);

    set_color(r, 57, 255, 20);
    draw_text(r, "WHAT YOU SEE", right, q.y + 350, 1.35F);
    set_color(r, 210, 221, 223);
    draw_wrapped_text(r,
        "LEFT: REALISTIC PROTOTYPE PACKAGE AND ITS SCHEMATIC SYMBOL. CENTER: PHYSICAL BOARD. SMALL SYMBOL BADGE BESIDE EACH PART: THE SAME PART IN SCHEMATIC LANGUAGE. RIGHT TOP: LIVE SCHEMATIC MIRROR. RIGHT MIDDLE: ONE-SENTENCE CODEX AND CONNECTION GUIDANCE.",
        right, q.y + 378, column_w, 1.0F, 5.0F);

    set_color(r, 122, 141, 145);
    draw_wrapped_text(r,
        "PROTOTYPE CONNECTION LISTS ARE LEARNING GUIDANCE. FINAL ELECTRICAL PIN COMPATIBILITY MUST COME FROM VERIFIED COMPONENT DATA AND ENGINEERING CHECKS.",
        q.x + 30, q.y + q.h - 62, q.w - 60, 0.9F, 4.0F);
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
    const std::array<float, 6> ys{142, 210, 278, 346, 414, 482};
    for (std::size_t i = 0; i < ys.size(); ++i) {
        if (contains({14, ys[i], pw - 28, 58}, x, y)) {
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
    q.y = std::clamp(q.y, board.y + 24, board.y + board.h - q.h - 8);
    s.parts.push_back({s.selected_kind, q});
    s.board_focus = s.parts.size() - 1;
    s.focus = FocusZone::BoardPart;
    s.validation_ran = false;
    s.status = std::string("PLACED ") + reference_for(s.parts, s.board_focus) + ". SYMBOL ADDED TO SCHEMATIC MIRROR.";
}

void connect_part(WorkbenchState& s, std::size_t index) {
    if (index >= s.parts.size()) return;
    if (!s.has_wire_start) {
        s.has_wire_start = true;
        s.wire_start = index;
        s.board_focus = index;
        s.focus = FocusZone::BoardPart;
        s.status = std::string("CONNECT FROM ") + reference_for(s.parts, index) + ". CHOOSE A SECOND PART.";
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
        const std::string from = reference_for(s.parts, s.wire_start);
        const std::string to = reference_for(s.parts, index);
        s.wires.push_back({s.wire_start, index});
        s.validation_ran = false;
        s.status = "CONNECTED " + from + " + " + to + ". SCHEMATIC WIRE UPDATED.";
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
    s.show_help = false;
    s.status = "BOARD CLEARED - CHOOSE A PART.";
}

void cycle_focus(WorkbenchState& s, int direction) {
    const std::array<FocusZone, 5> zones{
        FocusZone::Palette, FocusZone::BoardPart, FocusZone::Validate,
        FocusZone::Clear, FocusZone::Help};
    int current = 0;
    for (int i = 0; i < static_cast<int>(zones.size()); ++i) {
        if (zones[static_cast<std::size_t>(i)] == s.focus) current = i;
    }
    for (int tries = 0; tries < 5; ++tries) {
        current = (current + direction + 5) % 5;
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
        s.status = std::string("BOARD FOCUS ") + reference_for(s.parts, s.board_focus) + ". ENTER TO START OR FINISH A CONNECTION.";
    } else if (s.focus == FocusZone::Validate || s.focus == FocusZone::Clear || s.focus == FocusZone::Help) {
        cycle_focus(s, direction);
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
        case FocusZone::Help:
            s.show_help = !s.show_help;
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
                else if (key == SDLK_h || key == SDLK_F1) state.show_help = !state.show_help;
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
                    if (contains(help_button_for(w), x, y)) {
                        state.focus = FocusZone::Help;
                        state.show_help = !state.show_help;
                    } else if (contains(validate_button_for(w), x, y)) {
                        state.focus = FocusZone::Validate;
                        run_validation(state);
                    } else if (contains(clear_button_for(w), x, y)) {
                        state.focus = FocusZone::Clear;
                        clear_board(state);
                    } else if (click_compatible_target(state, x, y, w, h)) {
                        // The inspector target list starts or completes a connection.
                    } else if (!pick_palette(state, x, y, w)) {
                        std::size_t index = 0;
                        if (find_part(state, x, y, index)) {
                            state.focus = FocusZone::BoardPart;
                            state.board_focus = index;
                            state.status = std::string("BOARD FOCUS ") + reference_for(state.parts, index) + ". RIGHT CLICK OR ENTER TO CONNECT.";
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
