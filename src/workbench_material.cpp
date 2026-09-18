#define main formfactor_classic_workbench_main
#include "workbench.cpp"
#undef main

namespace {

struct MaterialVec3 {
    float x;
    float y;
    float z;
};

struct MaterialPoint {
    float x;
    float y;
};

struct MaterialCamera {
    float yaw{-0.16F};
    float pitch{0.82F};
    float zoom{1.0F};
    float pan_x{60.0F};
    float pan_y{8.0F};
};

enum class MaterialStep {
    PlaceLed = 0,
    SelectPower,
    SelectLed,
    Complete
};

struct MaterialSession {
    WorkbenchState board;
    MaterialStep step{MaterialStep::PlaceLed};
    MaterialCamera camera{};
    bool show_help{false};
    bool inspect_mode{true};
    bool middle_drag{false};
    int last_mouse_x{0};
    int last_mouse_y{0};
};

constexpr RectF kMaterialBoard{0.0F, 0.0F, 1000.0F, 620.0F};
constexpr float kMaterialPowerX = 330.0F;
constexpr float kMaterialPowerY = 365.0F;
constexpr float kMaterialLedX = 655.0F;
constexpr float kMaterialLedY = 320.0F;

float material_part_height(PartKind kind) {
    switch (kind) {
        case PartKind::Resistor: return 0.14F;
        case PartKind::Capacitor: return 0.28F;
        case PartKind::Led: return 0.30F;
        case PartKind::Chip: return 0.18F;
        case PartKind::Power: return 0.26F;
        case PartKind::Connector: return 0.22F;
    }
    return 0.16F;
}

MaterialVec3 material_world(float x, float y, float z) {
    const float nx = ((x - kMaterialBoard.x) / kMaterialBoard.w - 0.5F) * 2.8F;
    const float ny = ((y - kMaterialBoard.y) / kMaterialBoard.h - 0.5F) * 1.72F;
    return {nx, ny, z};
}

MaterialPoint material_project(const MaterialVec3& p, int width, int height,
                               const MaterialCamera& camera) {
    const float cy = std::cos(camera.yaw);
    const float sy = std::sin(camera.yaw);
    const float cp = std::cos(camera.pitch);
    const float sp = std::sin(camera.pitch);

    const float rx = p.x * cy - p.y * sy;
    const float ry = p.x * sy + p.y * cy;
    const float py = ry * cp - p.z * sp;
    const float pz = ry * sp + p.z * cp;
    const float distance = std::max(1.8F, 4.1F + pz);
    const float base_scale = std::min(static_cast<float>(width),
                                      static_cast<float>(height)) * 0.94F;
    const float scale = base_scale * camera.zoom / distance;

    return {
        static_cast<float>(width) * 0.63F + camera.pan_x + rx * scale,
        static_cast<float>(height) * 0.54F + camera.pan_y + py * scale
    };
}

MaterialPoint material_project_logical(float x, float y, float z, int width, int height,
                                       const MaterialCamera& camera) {
    return material_project(material_world(x, y, z), width, height, camera);
}

void material_line(SDL_Renderer* renderer, const MaterialPoint& a, const MaterialPoint& b) {
    SDL_RenderDrawLineF(renderer, a.x, a.y, b.x, b.y);
}

MaterialPoint material_mix(const MaterialPoint& a, const MaterialPoint& b, float t) {
    return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}

void material_fill_quad(SDL_Renderer* renderer, const MaterialPoint& a, const MaterialPoint& b,
                        const MaterialPoint& c, const MaterialPoint& d, int slices = 72) {
    for (int i = 0; i <= slices; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(slices);
        material_line(renderer, material_mix(a, d, t), material_mix(b, c, t));
    }
}

void material_fill_quad_gradient(SDL_Renderer* renderer, const MaterialPoint& a,
                                 const MaterialPoint& b, const MaterialPoint& c,
                                 const MaterialPoint& d,
                                 std::array<std::uint8_t, 3> near_color,
                                 std::array<std::uint8_t, 3> far_color,
                                 int slices = 96) {
    for (int i = 0; i <= slices; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(slices);
        const auto mix_channel = [t](std::uint8_t near_value, std::uint8_t far_value) {
            return static_cast<std::uint8_t>(
                static_cast<float>(near_value) +
                (static_cast<float>(far_value) - static_cast<float>(near_value)) * t);
        };
        set_color(renderer,
                  mix_channel(near_color[0], far_color[0]),
                  mix_channel(near_color[1], far_color[1]),
                  mix_channel(near_color[2], far_color[2]), 255);
        material_line(renderer, material_mix(a, d, t), material_mix(b, c, t));
    }
}

void material_soft_disc(SDL_Renderer* renderer, float x, float y, float radius,
                        std::array<std::uint8_t, 3> color,
                        std::uint8_t peak_alpha, int layers = 6) {
    for (int layer = layers; layer >= 1; --layer) {
        const float t = static_cast<float>(layer) / static_cast<float>(layers);
        const float layer_radius = radius * (0.52F + 0.48F * t);
        const std::uint8_t alpha = static_cast<std::uint8_t>(
            static_cast<float>(peak_alpha) * (1.0F - 0.76F * t));
        set_color(renderer, color[0], color[1], color[2], alpha);
        fill_circle(renderer, x, y, layer_radius);
    }
}

void material_outline_quad(SDL_Renderer* renderer, const MaterialPoint& a,
                           const MaterialPoint& b, const MaterialPoint& c,
                           const MaterialPoint& d) {
    material_line(renderer, a, b);
    material_line(renderer, b, c);
    material_line(renderer, c, d);
    material_line(renderer, d, a);
}

void material_reset_camera(MaterialCamera& camera) {
    camera = MaterialCamera{};
}

void material_reset(MaterialSession& session) {
    session.board = WorkbenchState{};
    session.board.parts.clear();
    session.board.wires.clear();
    session.board.show_help = false;
    session.board.selected_kind = PartKind::Led;
    session.board.palette_focus = static_cast<std::size_t>(PartKind::Led);
    session.board.parts.push_back(
        {PartKind::Power, part_rect(PartKind::Power, kMaterialPowerX, kMaterialPowerY)});
    session.board.board_focus = 0;
    session.board.focus = FocusZone::BoardPart;
    session.board.status = "STEP 1 - CLICK THE GLOWING LED TARGET. IT SNAPS INTO PLACE.";
    session.step = MaterialStep::PlaceLed;
}

void material_draw_room(SDL_Renderer* renderer, int width, int height) {
    set_color(renderer, 3, 6, 8);
    SDL_RenderClear(renderer);

    set_color(renderer, 7, 11, 14);
    fill_rect(renderer, {0.0F, 72.0F, static_cast<float>(width),
                         static_cast<float>(height) - 72.0F});

    const float horizon = static_cast<float>(height) * 0.34F;
    set_color(renderer, 17, 27, 31);
    for (int i = 1; i <= 14; ++i) {
        const float t = static_cast<float>(i) / 14.0F;
        const float y = horizon + t * t * (static_cast<float>(height) - horizon);
        SDL_RenderDrawLineF(renderer, 300.0F, y, static_cast<float>(width), y);
    }

    const float vanish_x = static_cast<float>(width) * 0.65F;
    for (int i = -9; i <= 9; ++i) {
        const float bottom_x = vanish_x + static_cast<float>(i) * 105.0F;
        SDL_RenderDrawLineF(renderer, vanish_x, horizon, bottom_x,
                            static_cast<float>(height));
    }

    set_color(renderer, 10, 18, 21, 245);
    fill_rect(renderer, {static_cast<float>(width) - 290.0F, 98.0F, 244.0F, 118.0F});
    set_color(renderer, 57, 255, 20, 255);
    draw_rect(renderer, {static_cast<float>(width) - 290.0F, 98.0F, 244.0F, 118.0F});
    draw_text(renderer, "LAB POWER", static_cast<float>(width) - 266.0F, 116.0F, 1.1F);
    set_color(renderer, 118, 139, 143, 255);
    draw_text(renderer, "TRAINING BENCH ONLINE", static_cast<float>(width) - 266.0F,
              145.0F, 0.78F);
    set_color(renderer, 57, 255, 20, 255);
    fill_circle(renderer, static_cast<float>(width) - 259.0F, 184.0F, 5.0F);
    set_color(renderer, 115, 132, 136, 255);
    draw_text(renderer, "READY", static_cast<float>(width) - 243.0F, 177.0F, 0.8F);
}

void material_draw_board(SDL_Renderer* renderer, int width, int height,
                         const MaterialCamera& camera) {
    const MaterialPoint a = material_project_logical(0.0F, 0.0F, 0.0F, width, height, camera);
    const MaterialPoint b = material_project_logical(1000.0F, 0.0F, 0.0F, width, height, camera);
    const MaterialPoint c = material_project_logical(1000.0F, 620.0F, 0.0F, width, height, camera);
    const MaterialPoint d = material_project_logical(0.0F, 620.0F, 0.0F, width, height, camera);
    const MaterialPoint bb = material_project_logical(1000.0F, 0.0F, -0.12F, width, height, camera);
    const MaterialPoint cb = material_project_logical(1000.0F, 620.0F, -0.12F, width, height, camera);
    const MaterialPoint db = material_project_logical(0.0F, 620.0F, -0.12F, width, height, camera);

    material_fill_quad_gradient(renderer, d, c, cb, db,
                                {8, 25, 20}, {2, 10, 9}, 96);
    material_fill_quad_gradient(renderer, b, c, cb, bb,
                                {12, 36, 29}, {3, 14, 12}, 96);

    material_fill_quad_gradient(renderer, a, b, c, d,
                                {20, 74, 53}, {5, 31, 24}, 160);
    set_color(renderer, 120, 176, 150, 210);
    material_outline_quad(renderer, a, b, c, d);
    set_color(renderer, 205, 235, 221, 75);
    material_line(renderer, a, b);

    set_color(renderer, 24, 72, 54, 88);
    for (int x = 100; x < 1000; x += 100) {
        material_line(renderer,
                      material_project_logical(static_cast<float>(x), 0.0F, 0.008F,
                                               width, height, camera),
                      material_project_logical(static_cast<float>(x), 620.0F, 0.008F,
                                               width, height, camera));
    }
    for (int y = 80; y < 620; y += 80) {
        material_line(renderer,
                      material_project_logical(0.0F, static_cast<float>(y), 0.008F,
                                               width, height, camera),
                      material_project_logical(1000.0F, static_cast<float>(y), 0.008F,
                                               width, height, camera));
    }

    const std::array<std::pair<float, float>, 4> holes{{
        {48.0F, 48.0F}, {952.0F, 48.0F}, {48.0F, 572.0F}, {952.0F, 572.0F}
    }};
    for (const auto& hole : holes) {
        const MaterialPoint p = material_project_logical(hole.first, hole.second, 0.018F,
                                                         width, height, camera);
        set_color(renderer, 0, 4, 4, 135);
        fill_circle(renderer, p.x + 2.0F, p.y + 3.0F, 10.0F);
        set_color(renderer, 178, 139, 47, 255);
        fill_circle(renderer, p.x, p.y, 9.0F);
        set_color(renderer, 236, 214, 139, 210);
        draw_circle(renderer, p.x - 0.8F, p.y - 0.8F, 7.5F);
        set_color(renderer, 18, 23, 22, 255);
        fill_circle(renderer, p.x, p.y, 4.8F);
        set_color(renderer, 240, 246, 244, 100);
        draw_circle(renderer, p.x - 1.0F, p.y - 1.0F, 4.0F);
    }
}

MaterialPoint material_part_center(const PlacedPart& part, int width, int height,
                                   const MaterialCamera& camera) {
    const float cx = part.rect.x + part.rect.w * 0.5F;
    const float cy = part.rect.y + part.rect.h * 0.5F;
    return material_project_logical(cx, cy, material_part_height(part.kind),
                                    width, height, camera);
}

void material_draw_shadow(SDL_Renderer* renderer, const PlacedPart& part, int width, int height,
                          const MaterialCamera& camera) {
    const float cx = part.rect.x + part.rect.w * 0.5F;
    const float cy = part.rect.y + part.rect.h * 0.5F;
    const MaterialPoint p = material_project_logical(cx + 10.0F, cy + 12.0F, 0.018F,
                                                     width, height, camera);
    const float radius = part.kind == PartKind::Power ? 31.0F : 22.0F;
    material_soft_disc(renderer, p.x, p.y, radius, {0, 0, 0}, 138, 7);
}

void material_component_color(PartKind kind, std::uint8_t& red, std::uint8_t& green,
                              std::uint8_t& blue) {
    switch (kind) {
        case PartKind::Power: red = 47; green = 58; blue = 63; return;
        case PartKind::Led: red = 57; green = 220; blue = 48; return;
        case PartKind::Resistor: red = 199; green = 158; blue = 89; return;
        case PartKind::Capacitor: red = 39; green = 92; blue = 166; return;
        case PartKind::Chip: red = 29; green = 32; blue = 35; return;
        case PartKind::Connector: red = 62; green = 68; blue = 71; return;
    }
    red = 80;
    green = 80;
    blue = 80;
}

void material_draw_component(SDL_Renderer* renderer, const PlacedPart& part,
                             const std::string& reference, int width, int height,
                             const MaterialCamera& camera, bool highlighted,
                             bool powered) {
    const float x0 = part.rect.x;
    const float y0 = part.rect.y;
    const float x1 = part.rect.x + part.rect.w;
    const float y1 = part.rect.y + part.rect.h;
    const float z = material_part_height(part.kind);

    const MaterialPoint a0 = material_project_logical(x0, y0, 0.025F, width, height, camera);
    const MaterialPoint b0 = material_project_logical(x1, y0, 0.025F, width, height, camera);
    const MaterialPoint c0 = material_project_logical(x1, y1, 0.025F, width, height, camera);
    const MaterialPoint d0 = material_project_logical(x0, y1, 0.025F, width, height, camera);
    const MaterialPoint a1 = material_project_logical(x0, y0, z, width, height, camera);
    const MaterialPoint b1 = material_project_logical(x1, y0, z, width, height, camera);
    const MaterialPoint c1 = material_project_logical(x1, y1, z, width, height, camera);
    const MaterialPoint d1 = material_project_logical(x0, y1, z, width, height, camera);

    std::uint8_t red = 80;
    std::uint8_t green = 80;
    std::uint8_t blue = 80;
    material_component_color(part.kind, red, green, blue);

    set_color(renderer, static_cast<std::uint8_t>(red / 2),
              static_cast<std::uint8_t>(green / 2),
              static_cast<std::uint8_t>(blue / 2), 255);
    material_fill_quad(renderer, d0, c0, c1, d1, 30);
    set_color(renderer, static_cast<std::uint8_t>(red * 0.70F),
              static_cast<std::uint8_t>(green * 0.70F),
              static_cast<std::uint8_t>(blue * 0.70F), 255);
    material_fill_quad(renderer, b0, c0, c1, b1, 30);
    material_fill_quad_gradient(
        renderer, a1, b1, c1, d1,
        {static_cast<std::uint8_t>(std::min(255.0F, red * 1.18F)),
         static_cast<std::uint8_t>(std::min(255.0F, green * 1.18F)),
         static_cast<std::uint8_t>(std::min(255.0F, blue * 1.18F))},
        {static_cast<std::uint8_t>(red * 0.72F),
         static_cast<std::uint8_t>(green * 0.72F),
         static_cast<std::uint8_t>(blue * 0.72F)}, 48);
    set_color(renderer, 255, 255, 255, 68);
    material_line(renderer, a1, b1);

    set_color(renderer, highlighted ? 255 : 205,
              highlighted ? 230 : 214,
              highlighted ? 70 : 218, 255);
    material_outline_quad(renderer, a1, b1, c1, d1);
    material_line(renderer, a0, a1);
    material_line(renderer, b0, b1);
    material_line(renderer, c0, c1);
    material_line(renderer, d0, d1);

    const MaterialPoint center = material_part_center(part, width, height, camera);
    set_color(renderer, 236, 242, 243, 255);
    draw_text(renderer, reference, center.x - 11.0F, center.y - 31.0F, 0.9F);

    if (part.kind == PartKind::Power) {
        const MaterialPoint plus = material_project_logical(x0 + part.rect.w * 0.32F,
                                                            y0 + part.rect.h * 0.40F,
                                                            z + 0.018F, width, height, camera);
        const MaterialPoint minus = material_project_logical(x0 + part.rect.w * 0.68F,
                                                             y0 + part.rect.h * 0.40F,
                                                             z + 0.018F, width, height, camera);
        for (const MaterialPoint terminal : {plus, minus}) {
            set_color(renderer, 35, 39, 40, 180);
            fill_circle(renderer, terminal.x + 1.5F, terminal.y + 2.0F, 8.0F);
            set_color(renderer, 172, 179, 178, 255);
            fill_circle(renderer, terminal.x, terminal.y, 7.0F);
            set_color(renderer, 236, 241, 238, 210);
            draw_circle(renderer, terminal.x - 0.5F, terminal.y - 0.5F, 5.7F);
            set_color(renderer, 74, 78, 78, 255);
            SDL_RenderDrawLineF(renderer, terminal.x - 4.0F, terminal.y,
                                terminal.x + 4.0F, terminal.y);
            SDL_RenderDrawLineF(renderer, terminal.x, terminal.y - 4.0F,
                                terminal.x, terminal.y + 4.0F);
        }
        set_color(renderer, 231, 69, 59, 255);
        fill_circle(renderer, plus.x - 10.0F, plus.y + 12.0F, 3.8F);
        set_color(renderer, 22, 25, 27, 255);
        fill_circle(renderer, minus.x + 10.0F, minus.y + 12.0F, 3.8F);
    } else if (part.kind == PartKind::Led) {
        const float glow = powered
            ? 13.0F + 3.0F * std::sin(static_cast<float>(SDL_GetTicks()) * 0.009F)
            : 8.0F;
        if (powered) {
            material_soft_disc(renderer, center.x, center.y - 5.0F, glow + 24.0F,
                               {57, 255, 20}, 115, 8);
        }
        set_color(renderer, powered ? 61 : 31, powered ? 198 : 106,
                  powered ? 44 : 38, 255);
        fill_circle(renderer, center.x, center.y - 5.0F, glow + 2.0F);
        set_color(renderer, powered ? 149 : 72, powered ? 255 : 165,
                  powered ? 131 : 78, 225);
        fill_circle(renderer, center.x - 1.5F, center.y - 7.0F, glow - 2.0F);
        set_color(renderer, 236, 255, 231, powered ? 245 : 150);
        fill_circle(renderer, center.x - glow * 0.32F, center.y - glow * 0.38F,
                    std::max(2.2F, glow * 0.18F));
        set_color(renderer, 224, 255, 215, 235);
        draw_circle(renderer, center.x, center.y - 5.0F, glow + 2.0F);
    }
}

void material_draw_wire(SDL_Renderer* renderer, const WorkbenchState& board,
                        int width, int height, const MaterialCamera& camera,
                        bool powered) {
    for (const Wire& wire : board.wires) {
        if (wire.from >= board.parts.size() || wire.to >= board.parts.size()) continue;
        const MaterialPoint a = material_part_center(board.parts[wire.from], width, height, camera);
        const MaterialPoint b = material_part_center(board.parts[wire.to], width, height, camera);

        set_color(renderer, 74, 39, 18, 255);
        SDL_RenderDrawLineF(renderer, a.x + 2.0F, a.y + 3.0F, b.x + 2.0F, b.y + 3.0F);
        set_color(renderer, 255, 139, 28, 255);
        SDL_RenderDrawLineF(renderer, a.x, a.y, b.x, b.y);
        SDL_RenderDrawLineF(renderer, a.x + 1.0F, a.y, b.x + 1.0F, b.y);

        if (powered) {
            const float phase = std::fmod(static_cast<float>(SDL_GetTicks()) * 0.00055F, 1.0F);
            const MaterialPoint pulse = material_mix(a, b, phase);
            set_color(renderer, 255, 235, 82, 210);
            fill_circle(renderer, pulse.x, pulse.y, 5.0F);
        }
    }
}

void material_target_hint(SDL_Renderer* renderer, int width, int height,
                          const MaterialCamera& camera) {
    const MaterialPoint target = material_project_logical(kMaterialLedX, kMaterialLedY, 0.04F,
                                                          width, height, camera);
    const float pulse = 8.0F + 3.0F * std::sin(static_cast<float>(SDL_GetTicks()) * 0.006F);
    set_color(renderer, 57, 255, 20, 210);
    draw_circle(renderer, target.x, target.y, 32.0F + pulse);
    draw_circle(renderer, target.x, target.y, 45.0F + pulse);
    draw_text(renderer, "PLACE LED HERE", target.x - 64.0F, target.y - 72.0F, 0.9F);
}

void material_part_hint(SDL_Renderer* renderer, const PlacedPart& part,
                        const std::string& label, int width, int height,
                        const MaterialCamera& camera) {
    const MaterialPoint p = material_part_center(part, width, height, camera);
    const float pulse = 7.0F + 3.0F * std::sin(static_cast<float>(SDL_GetTicks()) * 0.006F);
    set_color(renderer, 255, 225, 70, 235);
    draw_circle(renderer, p.x, p.y, 34.0F + pulse);
    draw_text(renderer, label, p.x - 54.0F, p.y - 61.0F, 0.82F);
}

int material_pick_part(const WorkbenchState& board, float mouse_x, float mouse_y,
                       int width, int height, const MaterialCamera& camera) {
    int best = -1;
    float best_distance = 78.0F * 78.0F;
    for (std::size_t i = 0; i < board.parts.size(); ++i) {
        const MaterialPoint p = material_part_center(board.parts[i], width, height, camera);
        const float dx = mouse_x - p.x;
        const float dy = mouse_y - p.y;
        const float distance = dx * dx + dy * dy;
        if (distance < best_distance) {
            best_distance = distance;
            best = static_cast<int>(i);
        }
    }
    return best;
}

bool material_near_led(float mouse_x, float mouse_y, int width, int height,
                       const MaterialCamera& camera) {
    const MaterialPoint target = material_project_logical(kMaterialLedX, kMaterialLedY, 0.04F,
                                                          width, height, camera);
    const float dx = mouse_x - target.x;
    const float dy = mouse_y - target.y;
    return dx * dx + dy * dy <= 150.0F * 150.0F;
}

void material_draw_step(SDL_Renderer* renderer, const std::string& number,
                        const std::string& text, float y, bool active, bool done) {
    if (done) set_color(renderer, 57, 255, 20, 255);
    else if (active) set_color(renderer, 255, 225, 70, 255);
    else set_color(renderer, 100, 118, 122, 255);
    draw_text(renderer, number, 24.0F, y, 1.15F);
    draw_wrapped_text(renderer, text, 56.0F, y, 218.0F, 0.92F, 4.0F);
}

void material_draw_ui(SDL_Renderer* renderer, int width, int height,
                      const MaterialSession& session) {
    set_color(renderer, 10, 15, 17, 248);
    fill_rect(renderer, {0.0F, 72.0F, 300.0F, static_cast<float>(height) - 72.0F});
    set_color(renderer, 57, 255, 20, 255);
    SDL_RenderDrawLineF(renderer, 299.0F, 72.0F, 299.0F, static_cast<float>(height));

    draw_text(renderer, "BEGINNER BUILD", 20.0F, 96.0F, 1.55F);
    set_color(renderer, 151, 169, 173, 255);
    draw_text(renderer, "FIRST CIRCUIT - 3 CLICKS", 20.0F, 123.0F, 0.9F);

    const int step = static_cast<int>(session.step);
    material_draw_step(renderer, "1", "LEFT CLICK THE GLOWING LED SLOT. SNAP DOES THE PRECISION WORK.",
                       168.0F, step == 0, step > 0);
    material_draw_step(renderer, "2", "RIGHT CLICK THE POWER BLOCK.",
                       254.0F, step == 1, step > 1);
    material_draw_step(renderer, "3", "RIGHT CLICK THE LED. THE WIRE IS BUILT AND TESTED.",
                       322.0F, step == 2, step > 2);
    material_draw_step(renderer, "4", "WATCH THE LIVE LED AND CURRENT PULSE AFTER THE CHECK PASSES.",
                       408.0F, step == 3, false);

    set_color(renderer, 57, 255, 20, 255);
    draw_text(renderer, "STATUS", 20.0F, 505.0F, 1.0F);
    set_color(renderer, 224, 232, 234, 255);
    draw_wrapped_text(renderer, session.board.status, 20.0F, 528.0F, 255.0F, 0.88F, 4.0F);

    set_color(renderer, 119, 138, 142, 255);
    draw_wrapped_text(renderer,
        "MIDDLE MOUSE DRAG ORBITS | W A S D PAN | Q E ORBIT | WHEEL ZOOM | R RESET",
        20.0F, static_cast<float>(height) - 118.0F, 260.0F, 0.76F, 4.0F);
    draw_wrapped_text(renderer,
        "I INSPECT PANEL | C RESET BUILD | H HELP | F2 CLASSIC 2D | ESC QUIT",
        20.0F, static_cast<float>(height) - 69.0F, 260.0F, 0.76F, 4.0F);

    set_color(renderer, 13, 20, 22, 255);
    fill_rect(renderer, {0.0F, 0.0F, static_cast<float>(width), 72.0F});
    set_color(renderer, 57, 255, 20, 255);
    fill_rect(renderer, {0.0F, 69.0F, static_cast<float>(width), 3.0F});
    draw_text(renderer, "FORMFACTOR 1.127", 20.0F, 16.0F, 2.0F);
    set_color(renderer, 169, 184, 188, 255);
    draw_text(renderer, "MATERIAL 3D LAB - DEPTH, LIGHT, LIVE CIRCUIT FEEDBACK", 20.0F, 46.0F, 0.94F);
}

void material_draw_inspector(SDL_Renderer* renderer, int width, int height,
                             const MaterialSession& session) {
    if (!session.inspect_mode) return;
    const RectF panel{static_cast<float>(width) - 302.0F,
                      static_cast<float>(height) - 292.0F, 270.0F, 252.0F};
    set_color(renderer, 5, 10, 12, 238);
    fill_rect(renderer, panel);
    set_color(renderer, 57, 255, 20, 255);
    draw_rect(renderer, panel);
    draw_text(renderer, "LIVE INSPECT", panel.x + 18.0F, panel.y + 18.0F, 1.25F);

    set_color(renderer, 181, 197, 201, 255);
    draw_wrapped_text(renderer,
        "TRAINING PARTS ARE IDEALIZED. THE GAME DOES NOT INVENT MISSING REAL-WORLD RATINGS.",
        panel.x + 18.0F, panel.y + 50.0F, panel.w - 36.0F, 0.74F, 4.0F);

    set_color(renderer, 128, 148, 152, 255);
    draw_text(renderer, "POWER SOURCE", panel.x + 18.0F, panel.y + 112.0F, 0.82F);
    draw_text(renderer, "LED", panel.x + 18.0F, panel.y + 137.0F, 0.82F);
    draw_text(renderer, "WIRE", panel.x + 18.0F, panel.y + 162.0F, 0.82F);
    draw_text(renderer, "VALIDATION", panel.x + 18.0F, panel.y + 187.0F, 0.82F);

    const bool has_led = session.board.parts.size() > 1;
    const bool has_wire = !session.board.wires.empty();
    set_color(renderer, 57, 255, 20, 255);
    draw_text(renderer, "READY", panel.x + 155.0F, panel.y + 112.0F, 0.82F);
    draw_text(renderer, has_led ? "PLACED" : "WAITING", panel.x + 155.0F, panel.y + 137.0F, 0.82F);
    draw_text(renderer, has_wire ? "LINKED" : "WAITING", panel.x + 155.0F, panel.y + 162.0F, 0.82F);
    draw_text(renderer, session.board.validation_passed ? "PASS" : "WAITING",
              panel.x + 155.0F, panel.y + 187.0F, 0.82F);

    set_color(renderer, 111, 129, 133, 255);
    draw_text(renderer, "PRESS I TO HIDE", panel.x + 18.0F, panel.y + 224.0F, 0.72F);
}

void material_draw_help(SDL_Renderer* renderer, int width, int height) {
    const RectF q{static_cast<float>(width) * 0.22F, static_cast<float>(height) * 0.15F,
                  static_cast<float>(width) * 0.62F, static_cast<float>(height) * 0.69F};
    set_color(renderer, 3, 7, 8, 248);
    fill_rect(renderer, q);
    set_color(renderer, 57, 255, 20, 255);
    draw_rect(renderer, q);
    draw_text(renderer, "FORMFACTOR 1.127 - MATERIAL 3D LAB", q.x + 28.0F, q.y + 28.0F, 1.55F);
    set_color(renderer, 221, 231, 233, 255);
    draw_wrapped_text(renderer,
        "THE BOARD NOW HAS A CLEAN BLANK FACE, DARK SOLDER-MASK DEPTH, PLATED HOLES, SOFTER CONTACT SHADOWS, METAL TERMINALS, SPECULAR HIGHLIGHTS, A GLASS-LIKE LED LENS, AND A MOVING CURRENT MARKER AFTER VALIDATION. THESE ARE VISUAL CUES. THEY DO NOT REPLACE THE ENGINEERING TRUTH LAYER.",
        q.x + 28.0F, q.y + 78.0F, q.w - 56.0F, 0.92F, 5.0F);
    set_color(renderer, 255, 225, 70, 255);
    draw_text(renderer, "CAMERA", q.x + 28.0F, q.y + 235.0F, 1.2F);
    set_color(renderer, 207, 219, 222, 255);
    draw_wrapped_text(renderer,
        "HOLD THE MIDDLE MOUSE BUTTON AND DRAG TO ORBIT. W A S D PAN. Q AND E ORBIT. THE MOUSE WHEEL ZOOMS. R RETURNS TO THE DEFAULT VIEW.",
        q.x + 28.0F, q.y + 265.0F, q.w - 56.0F, 0.9F, 5.0F);
    set_color(renderer, 255, 225, 70, 255);
    draw_text(renderer, "INSPECT", q.x + 28.0F, q.y + 355.0F, 1.2F);
    set_color(renderer, 207, 219, 222, 255);
    draw_wrapped_text(renderer,
        "PRESS I TO SHOW OR HIDE THE LIVE INSPECT PANEL. IT REPORTS ONLY WHAT THE CURRENT TRAINING BUILD REALLY KNOWS.",
        q.x + 28.0F, q.y + 385.0F, q.w - 56.0F, 0.9F, 5.0F);
    set_color(renderer, 135, 153, 157, 255);
    draw_text(renderer, "PRESS H TO CLOSE", q.x + 28.0F, q.y + q.h - 42.0F, 0.9F);
}

void material_render(SDL_Renderer* renderer, int width, int height,
                     const MaterialSession& session) {
    material_draw_room(renderer, width, height);
    material_draw_board(renderer, width, height, session.camera);

    for (const PlacedPart& part : session.board.parts) {
        material_draw_shadow(renderer, part, width, height, session.camera);
    }

    const bool powered = session.board.validation_passed &&
                         session.step == MaterialStep::Complete;
    material_draw_wire(renderer, session.board, width, height, session.camera, powered);

    for (std::size_t i = 0; i < session.board.parts.size(); ++i) {
        const bool highlighted =
            (session.step == MaterialStep::SelectPower &&
             session.board.parts[i].kind == PartKind::Power) ||
            (session.step == MaterialStep::SelectLed &&
             session.board.parts[i].kind == PartKind::Led) ||
            session.step == MaterialStep::Complete;
        material_draw_component(renderer, session.board.parts[i],
                                reference_for(session.board.parts, i), width, height,
                                session.camera, highlighted, powered);
    }

    if (session.step == MaterialStep::PlaceLed) {
        material_target_hint(renderer, width, height, session.camera);
    } else if (session.step == MaterialStep::SelectPower && !session.board.parts.empty()) {
        material_part_hint(renderer, session.board.parts[0], "RIGHT CLICK POWER",
                           width, height, session.camera);
    } else if (session.step == MaterialStep::SelectLed && session.board.parts.size() > 1) {
        material_part_hint(renderer, session.board.parts[1], "RIGHT CLICK LED",
                           width, height, session.camera);
    } else if (session.step == MaterialStep::Complete) {
        const float pulse = 0.5F + 0.5F * std::sin(static_cast<float>(SDL_GetTicks()) * 0.004F);
        const float box_w = std::min(550.0F, static_cast<float>(width) - 360.0F);
        const RectF success{static_cast<float>(width) * 0.43F, 92.0F, box_w, 68.0F};
        set_color(renderer, 39, static_cast<std::uint8_t>(205 + 50 * pulse), 31, 245);
        fill_rect(renderer, success);
        set_color(renderer, 2, 13, 7, 255);
        draw_text(renderer, "CIRCUIT LIVE", success.x + 24.0F, success.y + 15.0F, 2.0F);
        draw_text(renderer, "VALIDATION PASS - VISUAL CURRENT FEEDBACK ACTIVE",
                  success.x + 24.0F, success.y + 45.0F, 0.76F);
    }

    material_draw_ui(renderer, width, height, session);
    material_draw_inspector(renderer, width, height, session);
    if (session.show_help) material_draw_help(renderer, width, height);
    SDL_RenderPresent(renderer);
}

}  // namespace

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "FormFactor 1.127 - Material 3D Lab", SDL_WINDOWPOS_CENTERED,
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
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    MaterialSession session;
    material_reset(session);

    bool running = true;
    bool launch_classic = false;
    while (running) {
        SDL_Event event{};
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
                continue;
            }

            if (event.type == SDL_MOUSEWHEEL) {
                session.camera.zoom = std::clamp(
                    session.camera.zoom + static_cast<float>(event.wheel.y) * 0.08F,
                    0.68F, 1.75F);
                continue;
            }

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_MIDDLE) {
                session.middle_drag = true;
                session.last_mouse_x = event.button.x;
                session.last_mouse_y = event.button.y;
                continue;
            }

            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_MIDDLE) {
                session.middle_drag = false;
                continue;
            }

            if (event.type == SDL_MOUSEMOTION && session.middle_drag) {
                const int dx = event.motion.x - session.last_mouse_x;
                const int dy = event.motion.y - session.last_mouse_y;
                session.last_mouse_x = event.motion.x;
                session.last_mouse_y = event.motion.y;
                session.camera.yaw += static_cast<float>(dx) * 0.006F;
                session.camera.pitch = std::clamp(
                    session.camera.pitch - static_cast<float>(dy) * 0.005F,
                    0.35F, 1.18F);
                continue;
            }

            if (event.type == SDL_KEYDOWN) {
                const SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE) {
                    running = false;
                } else if (key == SDLK_h || key == SDLK_F1) {
                    session.show_help = !session.show_help;
                } else if (key == SDLK_i) {
                    session.inspect_mode = !session.inspect_mode;
                } else if (key == SDLK_F2) {
                    launch_classic = true;
                    running = false;
                } else if (key == SDLK_c) {
                    material_reset(session);
                } else if (key == SDLK_r) {
                    material_reset_camera(session.camera);
                } else if (key == SDLK_a) {
                    session.camera.pan_x -= 24.0F;
                } else if (key == SDLK_d) {
                    session.camera.pan_x += 24.0F;
                } else if (key == SDLK_w) {
                    session.camera.pan_y -= 20.0F;
                } else if (key == SDLK_s) {
                    session.camera.pan_y += 20.0F;
                } else if (key == SDLK_q) {
                    session.camera.yaw -= 0.08F;
                } else if (key == SDLK_e) {
                    session.camera.yaw += 0.08F;
                } else if (key == SDLK_UP) {
                    session.camera.pitch = std::clamp(session.camera.pitch + 0.06F,
                                                      0.35F, 1.18F);
                } else if (key == SDLK_DOWN) {
                    session.camera.pitch = std::clamp(session.camera.pitch - 0.06F,
                                                      0.35F, 1.18F);
                } else if (key == SDLK_v) {
                    run_validation(session.board);
                    if (session.board.validation_passed) session.step = MaterialStep::Complete;
                }
                continue;
            }

            if (event.type != SDL_MOUSEBUTTONDOWN || session.show_help) continue;

            int width = 0;
            int height = 0;
            SDL_GetRendererOutputSize(renderer, &width, &height);
            const float mouse_x = static_cast<float>(event.button.x);
            const float mouse_y = static_cast<float>(event.button.y);

            if (event.button.button == SDL_BUTTON_LEFT &&
                session.step == MaterialStep::PlaceLed) {
                if (material_near_led(mouse_x, mouse_y, width, height, session.camera)) {
                    session.board.parts.push_back(
                        {PartKind::Led, part_rect(PartKind::Led, kMaterialLedX, kMaterialLedY)});
                    session.board.board_focus = 1;
                    session.board.focus = FocusZone::BoardPart;
                    session.step = MaterialStep::SelectPower;
                    session.board.status = "NICE - LED SNAPPED IN. STEP 2 - RIGHT CLICK THE POWER BLOCK.";
                } else {
                    session.board.status = "FOLLOW THE GREEN RINGS. CLICK NEAR THE GLOWING LED TARGET.";
                }
                continue;
            }

            if (event.button.button == SDL_BUTTON_RIGHT) {
                const int picked = material_pick_part(session.board, mouse_x, mouse_y,
                                                      width, height, session.camera);
                if (picked < 0) {
                    session.board.status = "RIGHT CLICK THE GLOWING PART.";
                    continue;
                }

                const PartKind kind =
                    session.board.parts[static_cast<std::size_t>(picked)].kind;
                if (session.step == MaterialStep::SelectPower) {
                    if (kind != PartKind::Power) {
                        session.board.status = "START THE WIRE AT POWER. RIGHT CLICK THE POWER BLOCK.";
                        continue;
                    }
                    connect_part(session.board, static_cast<std::size_t>(picked));
                    session.step = MaterialStep::SelectLed;
                    session.board.status = "POWER SELECTED. STEP 3 - RIGHT CLICK THE LED.";
                } else if (session.step == MaterialStep::SelectLed) {
                    if (kind != PartKind::Led) {
                        session.board.status = "NOW RIGHT CLICK THE LED TO FINISH THE WIRE.";
                        continue;
                    }
                    connect_part(session.board, static_cast<std::size_t>(picked));
                    run_validation(session.board);
                    if (session.board.validation_passed) {
                        session.step = MaterialStep::Complete;
                        session.board.status = "PASS - THE LED IS LIVE AND THE CURRENT PATH NOW ANIMATES.";
                    }
                }
            }
        }

        int width = 0;
        int height = 0;
        SDL_GetRendererOutputSize(renderer, &width, &height);
        material_render(renderer, width, height, session);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    if (launch_classic) return formfactor_classic_workbench_main(argc, argv);
    return 0;
}
