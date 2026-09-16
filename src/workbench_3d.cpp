#define main formfactor_classic_workbench_main
#include "workbench.cpp"
#undef main

namespace {

struct Vec3 {
    float x;
    float y;
    float z;
};

struct ScreenPoint {
    float x;
    float y;
};

struct Camera3D {
    float yaw{-0.16F};
    float pitch{0.82F};
    float zoom{1.0F};
    float pan_x{60.0F};
    float pan_y{8.0F};
};

enum class TutorialStep {
    PlaceLed = 0,
    SelectPower,
    SelectLed,
    Complete
};

struct BeginnerSession {
    WorkbenchState board;
    TutorialStep step{TutorialStep::PlaceLed};
    Camera3D camera{};
    bool show_help{false};
};

constexpr RectF kLogicalBoard{0.0F, 0.0F, 1000.0F, 620.0F};
constexpr float kPowerX = 330.0F;
constexpr float kPowerY = 365.0F;
constexpr float kLedX = 655.0F;
constexpr float kLedY = 320.0F;

float part_height(PartKind kind) {
    switch (kind) {
        case PartKind::Resistor: return 0.14F;
        case PartKind::Capacitor: return 0.28F;
        case PartKind::Led: return 0.26F;
        case PartKind::Chip: return 0.18F;
        case PartKind::Power: return 0.24F;
        case PartKind::Connector: return 0.22F;
    }
    return 0.16F;
}

Vec3 logical_to_world(float x, float y, float z) {
    const float nx = ((x - kLogicalBoard.x) / kLogicalBoard.w - 0.5F) * 2.8F;
    const float ny = ((y - kLogicalBoard.y) / kLogicalBoard.h - 0.5F) * 1.72F;
    return {nx, ny, z};
}

ScreenPoint project_point(const Vec3& p, int width, int height, const Camera3D& camera) {
    const float cy = std::cos(camera.yaw);
    const float sy = std::sin(camera.yaw);
    const float cp = std::cos(camera.pitch);
    const float sp = std::sin(camera.pitch);

    const float rx = p.x * cy - p.y * sy;
    const float ry = p.x * sy + p.y * cy;
    const float rz = p.z;

    const float py = ry * cp - rz * sp;
    const float pz = ry * sp + rz * cp;
    const float distance = std::max(1.8F, 4.1F + pz);
    const float base_scale = std::min(static_cast<float>(width),
                                      static_cast<float>(height)) * 0.94F;
    const float scale = base_scale * camera.zoom / distance;

    return {
        static_cast<float>(width) * 0.63F + camera.pan_x + rx * scale,
        static_cast<float>(height) * 0.54F + camera.pan_y + py * scale
    };
}

ScreenPoint project_logical(float x, float y, float z, int width, int height,
                            const Camera3D& camera) {
    return project_point(logical_to_world(x, y, z), width, height, camera);
}

void draw_line(SDL_Renderer* r, const ScreenPoint& a, const ScreenPoint& b) {
    SDL_RenderDrawLineF(r, a.x, a.y, b.x, b.y);
}

ScreenPoint mix(const ScreenPoint& a, const ScreenPoint& b, float t) {
    return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t};
}

void fill_quad(SDL_Renderer* r, const ScreenPoint& a, const ScreenPoint& b,
               const ScreenPoint& c, const ScreenPoint& d, int slices = 72) {
    for (int i = 0; i <= slices; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(slices);
        const ScreenPoint left = mix(a, d, t);
        const ScreenPoint right = mix(b, c, t);
        draw_line(r, left, right);
    }
}

void outline_quad(SDL_Renderer* r, const ScreenPoint& a, const ScreenPoint& b,
                  const ScreenPoint& c, const ScreenPoint& d) {
    draw_line(r, a, b);
    draw_line(r, b, c);
    draw_line(r, c, d);
    draw_line(r, d, a);
}

void reset_camera(Camera3D& camera) {
    camera = Camera3D{};
}

void reset_beginner(BeginnerSession& session) {
    session.board = WorkbenchState{};
    session.board.parts.clear();
    session.board.wires.clear();
    session.board.show_help = false;
    session.board.selected_kind = PartKind::Led;
    session.board.palette_focus = static_cast<std::size_t>(PartKind::Led);
    session.board.parts.push_back({PartKind::Power, part_rect(PartKind::Power, kPowerX, kPowerY)});
    session.board.board_focus = 0;
    session.board.focus = FocusZone::BoardPart;
    session.board.status = "STEP 1 - CLICK THE GLOWING LED TARGET. IT WILL SNAP INTO PLACE.";
    session.step = TutorialStep::PlaceLed;
}

void draw_floor_3d(SDL_Renderer* r, int width, int height) {
    set_color(r, 5, 8, 10);
    SDL_RenderClear(r);
    set_color(r, 10, 15, 18);
    fill_rect(r, {0.0F, 72.0F, static_cast<float>(width), static_cast<float>(height) - 72.0F});

    const float horizon = static_cast<float>(height) * 0.34F;
    set_color(r, 21, 31, 34);
    for (int i = 1; i <= 12; ++i) {
        const float t = static_cast<float>(i) / 12.0F;
        const float y = horizon + t * t * (static_cast<float>(height) - horizon);
        SDL_RenderDrawLineF(r, 300.0F, y, static_cast<float>(width), y);
    }
    const float vanishing_x = static_cast<float>(width) * 0.65F;
    for (int i = -8; i <= 8; ++i) {
        const float bottom_x = vanishing_x + static_cast<float>(i) * 105.0F;
        SDL_RenderDrawLineF(r, vanishing_x, horizon, bottom_x, static_cast<float>(height));
    }
}

void draw_board_3d(SDL_Renderer* r, int width, int height, const Camera3D& camera) {
    const ScreenPoint a = project_logical(0.0F, 0.0F, 0.0F, width, height, camera);
    const ScreenPoint b = project_logical(1000.0F, 0.0F, 0.0F, width, height, camera);
    const ScreenPoint c = project_logical(1000.0F, 620.0F, 0.0F, width, height, camera);
    const ScreenPoint d = project_logical(0.0F, 620.0F, 0.0F, width, height, camera);

    const ScreenPoint ab = project_logical(0.0F, 0.0F, -0.10F, width, height, camera);
    const ScreenPoint bb = project_logical(1000.0F, 0.0F, -0.10F, width, height, camera);
    const ScreenPoint cb = project_logical(1000.0F, 620.0F, -0.10F, width, height, camera);
    const ScreenPoint db = project_logical(0.0F, 620.0F, -0.10F, width, height, camera);

    set_color(r, 5, 24, 20, 255);
    fill_quad(r, d, c, cb, db);
    set_color(r, 7, 31, 24, 255);
    fill_quad(r, b, c, cb, bb);

    set_color(r, 17, 67, 48, 255);
    fill_quad(r, a, b, c, d, 110);
    set_color(r, 57, 255, 20, 255);
    outline_quad(r, a, b, c, d);

    set_color(r, 31, 92, 67, 180);
    for (int x = 100; x < 1000; x += 100) {
        draw_line(r,
                  project_logical(static_cast<float>(x), 0.0F, 0.008F, width, height, camera),
                  project_logical(static_cast<float>(x), 620.0F, 0.008F, width, height, camera));
    }
    for (int y = 80; y < 620; y += 80) {
        draw_line(r,
                  project_logical(0.0F, static_cast<float>(y), 0.008F, width, height, camera),
                  project_logical(1000.0F, static_cast<float>(y), 0.008F, width, height, camera));
    }
}

void component_color(PartKind kind, std::uint8_t& red, std::uint8_t& green,
                     std::uint8_t& blue) {
    switch (kind) {
        case PartKind::Power: red = 45; green = 56; blue = 62; return;
        case PartKind::Led: red = 57; green = 220; blue = 48; return;
        case PartKind::Resistor: red = 199; green = 158; blue = 89; return;
        case PartKind::Capacitor: red = 39; green = 92; blue = 166; return;
        case PartKind::Chip: red = 29; green = 32; blue = 35; return;
        case PartKind::Connector: red = 62; green = 68; blue = 71; return;
    }
    red = 80; green = 80; blue = 80;
}

ScreenPoint part_screen_center(const PlacedPart& part, int width, int height,
                               const Camera3D& camera) {
    const float cx = part.rect.x + part.rect.w * 0.5F;
    const float cy = part.rect.y + part.rect.h * 0.5F;
    return project_logical(cx, cy, part_height(part.kind), width, height, camera);
}

void draw_component_3d(SDL_Renderer* r, const PlacedPart& part, const std::string& reference,
                       int width, int height, const Camera3D& camera, bool highlighted) {
    const float x0 = part.rect.x;
    const float y0 = part.rect.y;
    const float x1 = part.rect.x + part.rect.w;
    const float y1 = part.rect.y + part.rect.h;
    const float z = part_height(part.kind);

    const ScreenPoint a0 = project_logical(x0, y0, 0.025F, width, height, camera);
    const ScreenPoint b0 = project_logical(x1, y0, 0.025F, width, height, camera);
    const ScreenPoint c0 = project_logical(x1, y1, 0.025F, width, height, camera);
    const ScreenPoint d0 = project_logical(x0, y1, 0.025F, width, height, camera);
    const ScreenPoint a1 = project_logical(x0, y0, z, width, height, camera);
    const ScreenPoint b1 = project_logical(x1, y0, z, width, height, camera);
    const ScreenPoint c1 = project_logical(x1, y1, z, width, height, camera);
    const ScreenPoint d1 = project_logical(x0, y1, z, width, height, camera);

    std::uint8_t red = 80, green = 80, blue = 80;
    component_color(part.kind, red, green, blue);

    set_color(r, static_cast<std::uint8_t>(red / 2),
              static_cast<std::uint8_t>(green / 2),
              static_cast<std::uint8_t>(blue / 2), 255);
    fill_quad(r, d0, c0, c1, d1, 30);
    set_color(r, static_cast<std::uint8_t>(red * 0.70F),
              static_cast<std::uint8_t>(green * 0.70F),
              static_cast<std::uint8_t>(blue * 0.70F), 255);
    fill_quad(r, b0, c0, c1, b1, 30);
    set_color(r, red, green, blue, 255);
    fill_quad(r, a1, b1, c1, d1, 35);

    set_color(r, highlighted ? 255 : 205,
              highlighted ? 230 : 214,
              highlighted ? 70 : 218, 255);
    outline_quad(r, a1, b1, c1, d1);
    draw_line(r, a0, a1);
    draw_line(r, b0, b1);
    draw_line(r, c0, c1);
    draw_line(r, d0, d1);

    const ScreenPoint center = part_screen_center(part, width, height, camera);
    set_color(r, 236, 242, 243, 255);
    draw_text(r, reference, center.x - 11.0F, center.y - 31.0F, 0.9F);
    if (part.kind == PartKind::Power) {
        draw_text(r, "PWR", center.x - 13.0F, center.y - 4.0F, 0.8F);
    } else if (part.kind == PartKind::Led) {
        set_color(r, 210, 255, 202, 255);
        draw_circle(r, center.x, center.y - 4.0F, 8.0F);
    }
}

void draw_wires_3d(SDL_Renderer* r, const WorkbenchState& board, int width, int height,
                   const Camera3D& camera) {
    set_color(r, 255, 139, 28, 255);
    for (const Wire& wire : board.wires) {
        if (wire.from >= board.parts.size() || wire.to >= board.parts.size()) continue;
        const PlacedPart& from = board.parts[wire.from];
        const PlacedPart& to = board.parts[wire.to];
        const ScreenPoint a = part_screen_center(from, width, height, camera);
        const ScreenPoint b = part_screen_center(to, width, height, camera);
        SDL_RenderDrawLineF(r, a.x, a.y, b.x, b.y);
        SDL_RenderDrawLineF(r, a.x + 1.0F, a.y + 1.0F, b.x + 1.0F, b.y + 1.0F);
    }
}

void draw_target_hint(SDL_Renderer* r, int width, int height, const Camera3D& camera) {
    const ScreenPoint target = project_logical(kLedX, kLedY, 0.04F, width, height, camera);
    const float pulse = 8.0F + 3.0F * std::sin(static_cast<float>(SDL_GetTicks()) * 0.006F);
    set_color(r, 57, 255, 20, 210);
    draw_circle(r, target.x, target.y, 32.0F + pulse);
    draw_circle(r, target.x, target.y, 45.0F + pulse);
    draw_text(r, "PLACE LED HERE", target.x - 64.0F, target.y - 72.0F, 0.9F);
}

void draw_part_hint(SDL_Renderer* r, const PlacedPart& part, const std::string& label,
                    int width, int height, const Camera3D& camera) {
    const ScreenPoint p = part_screen_center(part, width, height, camera);
    const float pulse = 7.0F + 3.0F * std::sin(static_cast<float>(SDL_GetTicks()) * 0.006F);
    set_color(r, 255, 225, 70, 235);
    draw_circle(r, p.x, p.y, 34.0F + pulse);
    draw_text(r, label, p.x - 54.0F, p.y - 61.0F, 0.82F);
}

int pick_part_3d(const WorkbenchState& board, float mouse_x, float mouse_y,
                 int width, int height, const Camera3D& camera) {
    int best = -1;
    float best_distance = 78.0F * 78.0F;
    for (std::size_t i = 0; i < board.parts.size(); ++i) {
        const ScreenPoint p = part_screen_center(board.parts[i], width, height, camera);
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

void draw_step(SDL_Renderer* r, const std::string& number, const std::string& text,
               float y, bool active, bool done) {
    if (done) set_color(r, 57, 255, 20, 255);
    else if (active) set_color(r, 255, 225, 70, 255);
    else set_color(r, 100, 118, 122, 255);
    draw_text(r, number, 24.0F, y, 1.15F);
    draw_wrapped_text(r, text, 56.0F, y, 218.0F, 0.92F, 4.0F);
}

void draw_beginner_ui(SDL_Renderer* r, int width, int height, const BeginnerSession& session) {
    set_color(r, 10, 15, 17, 248);
    fill_rect(r, {0.0F, 72.0F, 300.0F, static_cast<float>(height) - 72.0F});
    set_color(r, 57, 255, 20, 255);
    SDL_RenderDrawLineF(r, 299.0F, 72.0F, 299.0F, static_cast<float>(height));

    draw_text(r, "BEGINNER BUILD", 20.0F, 96.0F, 1.55F);
    set_color(r, 151, 169, 173, 255);
    draw_text(r, "FIRST CIRCUIT - 3 CLICKS", 20.0F, 123.0F, 0.9F);

    const int step = static_cast<int>(session.step);
    draw_step(r, "1", "LEFT CLICK THE GLOWING LED SLOT. SNAP DOES THE PRECISION WORK.",
              168.0F, step == 0, step > 0);
    draw_step(r, "2", "RIGHT CLICK THE POWER BLOCK.", 254.0F, step == 1, step > 1);
    draw_step(r, "3", "RIGHT CLICK THE LED. THE WIRE IS BUILT AND TESTED.",
              322.0F, step == 2, step > 2);
    draw_step(r, "4", "CIRCUIT WORKS. THEN EXPLORE OR PRESS F2 FOR CLASSIC MODE.",
              408.0F, step == 3, false);

    set_color(r, 57, 255, 20, 255);
    draw_text(r, "STATUS", 20.0F, 505.0F, 1.0F);
    set_color(r, 224, 232, 234, 255);
    draw_wrapped_text(r, session.board.status, 20.0F, 528.0F, 255.0F, 0.88F, 4.0F);

    set_color(r, 119, 138, 142, 255);
    draw_wrapped_text(r,
        "CAMERA: W A S D PAN | Q E ORBIT | UP DOWN TILT | MOUSE WHEEL ZOOM | R RESET",
        20.0F, static_cast<float>(height) - 112.0F, 260.0F, 0.78F, 4.0F);
    draw_wrapped_text(r,
        "C RESET BUILD | H HELP | F2 CLASSIC 2D | ESC QUIT",
        20.0F, static_cast<float>(height) - 65.0F, 260.0F, 0.78F, 4.0F);

    set_color(r, 13, 20, 22, 255);
    fill_rect(r, {0.0F, 0.0F, static_cast<float>(width), 72.0F});
    set_color(r, 57, 255, 20, 255);
    fill_rect(r, {0.0F, 69.0F, static_cast<float>(width), 3.0F});
    draw_text(r, "FORMFACTOR 1.05", 20.0F, 16.0F, 2.0F);
    set_color(r, 169, 184, 188, 255);
    draw_text(r, "BEGINNER 3D LAB - BUILD FIRST, LEARN DEPTH LATER", 20.0F, 46.0F, 0.94F);
}

void draw_help_3d(SDL_Renderer* r, int width, int height) {
    const RectF q{static_cast<float>(width) * 0.22F, static_cast<float>(height) * 0.16F,
                  static_cast<float>(width) * 0.62F, static_cast<float>(height) * 0.66F};
    set_color(r, 3, 7, 8, 248);
    fill_rect(r, q);
    set_color(r, 57, 255, 20, 255);
    draw_rect(r, q);
    draw_text(r, "FORMFACTOR 1.05 - EASY START", q.x + 28.0F, q.y + 28.0F, 1.8F);
    set_color(r, 221, 231, 233, 255);
    draw_wrapped_text(r,
        "THE FIRST BUILD IS INTENTIONALLY SIMPLE. CLICK THE GLOWING LED SLOT. THE PART SNAPS INTO THE CORRECT PLACE. RIGHT CLICK POWER, THEN RIGHT CLICK LED. FORMFACTOR CONNECTS THEM AND RUNS THE FIRST TEST. YOU CAN FINISH THIS WITHOUT KNOWING PCB DESIGN.",
        q.x + 28.0F, q.y + 82.0F, q.w - 56.0F, 1.0F, 6.0F);
    set_color(r, 255, 225, 70, 255);
    draw_text(r, "3D CAMERA", q.x + 28.0F, q.y + 225.0F, 1.35F);
    set_color(r, 207, 219, 222, 255);
    draw_wrapped_text(r,
        "W A S D MOVE THE CAMERA. Q AND E ORBIT. UP AND DOWN CHANGE THE VIEW ANGLE. THE MOUSE WHEEL ZOOMS. R RETURNS TO THE DEFAULT VIEW.",
        q.x + 28.0F, q.y + 257.0F, q.w - 56.0F, 0.95F, 5.0F);
    set_color(r, 135, 153, 157, 255);
    draw_text(r, "PRESS H TO CLOSE", q.x + 28.0F, q.y + q.h - 42.0F, 0.9F);
}

void render_beginner(SDL_Renderer* r, int width, int height, const BeginnerSession& session) {
    draw_floor_3d(r, width, height);
    draw_board_3d(r, width, height, session.camera);
    draw_wires_3d(r, session.board, width, height, session.camera);

    for (std::size_t i = 0; i < session.board.parts.size(); ++i) {
        const bool highlighted =
            (session.step == TutorialStep::SelectPower && session.board.parts[i].kind == PartKind::Power) ||
            (session.step == TutorialStep::SelectLed && session.board.parts[i].kind == PartKind::Led) ||
            session.step == TutorialStep::Complete;
        draw_component_3d(r, session.board.parts[i], reference_for(session.board.parts, i),
                          width, height, session.camera, highlighted);
    }

    if (session.step == TutorialStep::PlaceLed) {
        draw_target_hint(r, width, height, session.camera);
    } else if (session.step == TutorialStep::SelectPower && !session.board.parts.empty()) {
        draw_part_hint(r, session.board.parts[0], "RIGHT CLICK POWER", width, height, session.camera);
    } else if (session.step == TutorialStep::SelectLed && session.board.parts.size() > 1) {
        draw_part_hint(r, session.board.parts[1], "RIGHT CLICK LED", width, height, session.camera);
    } else if (session.step == TutorialStep::Complete) {
        set_color(r, 57, 255, 20, 255);
        const float box_w = std::min(520.0F, static_cast<float>(width) - 360.0F);
        const RectF success{static_cast<float>(width) * 0.46F, 90.0F, box_w, 62.0F};
        fill_rect(r, success);
        set_color(r, 3, 12, 8, 255);
        draw_text(r, "CIRCUIT WORKS", success.x + 24.0F, success.y + 17.0F, 2.0F);
    }

    draw_beginner_ui(r, width, height, session);
    if (session.show_help) draw_help_3d(r, width, height);
    SDL_RenderPresent(r);
}

bool near_led_target(float mouse_x, float mouse_y, int width, int height,
                     const Camera3D& camera) {
    const ScreenPoint target = project_logical(kLedX, kLedY, 0.04F, width, height, camera);
    const float dx = mouse_x - target.x;
    const float dy = mouse_y - target.y;
    return dx * dx + dy * dy <= 150.0F * 150.0F;
}

}  // namespace

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "FormFactor 1.05 - Beginner 3D Lab", SDL_WINDOWPOS_CENTERED,
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

    BeginnerSession session;
    reset_beginner(session);

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

            if (event.type == SDL_KEYDOWN) {
                const SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE) {
                    running = false;
                } else if (key == SDLK_h || key == SDLK_F1) {
                    session.show_help = !session.show_help;
                } else if (key == SDLK_F2) {
                    launch_classic = true;
                    running = false;
                } else if (key == SDLK_c) {
                    reset_beginner(session);
                } else if (key == SDLK_r) {
                    reset_camera(session.camera);
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
                    session.camera.pitch = std::clamp(session.camera.pitch + 0.06F, 0.35F, 1.18F);
                } else if (key == SDLK_DOWN) {
                    session.camera.pitch = std::clamp(session.camera.pitch - 0.06F, 0.35F, 1.18F);
                } else if (key == SDLK_v) {
                    run_validation(session.board);
                    if (session.board.validation_passed) session.step = TutorialStep::Complete;
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
                session.step == TutorialStep::PlaceLed) {
                if (near_led_target(mouse_x, mouse_y, width, height, session.camera)) {
                    session.board.parts.push_back(
                        {PartKind::Led, part_rect(PartKind::Led, kLedX, kLedY)});
                    session.board.board_focus = 1;
                    session.board.focus = FocusZone::BoardPart;
                    session.step = TutorialStep::SelectPower;
                    session.board.status = "NICE - LED SNAPPED IN. STEP 2 - RIGHT CLICK THE POWER BLOCK.";
                } else {
                    session.board.status = "FOLLOW THE GREEN RINGS. CLICK NEAR THE GLOWING LED TARGET.";
                }
                continue;
            }

            if (event.button.button == SDL_BUTTON_RIGHT) {
                const int picked = pick_part_3d(session.board, mouse_x, mouse_y,
                                                width, height, session.camera);
                if (picked < 0) {
                    session.board.status = "RIGHT CLICK THE GLOWING PART.";
                    continue;
                }

                const PartKind kind = session.board.parts[static_cast<std::size_t>(picked)].kind;
                if (session.step == TutorialStep::SelectPower) {
                    if (kind != PartKind::Power) {
                        session.board.status = "START THE WIRE AT POWER. RIGHT CLICK THE POWER BLOCK.";
                        continue;
                    }
                    connect_part(session.board, static_cast<std::size_t>(picked));
                    session.step = TutorialStep::SelectLed;
                    session.board.status = "POWER SELECTED. STEP 3 - RIGHT CLICK THE LED.";
                } else if (session.step == TutorialStep::SelectLed) {
                    if (kind != PartKind::Led) {
                        session.board.status = "NOW RIGHT CLICK THE LED TO FINISH THE WIRE.";
                        continue;
                    }
                    connect_part(session.board, static_cast<std::size_t>(picked));
                    run_validation(session.board);
                    if (session.board.validation_passed) {
                        session.step = TutorialStep::Complete;
                        session.board.status = "PASS - YOUR FIRST CIRCUIT WORKS. THREE CLICKS, NO MANUAL REQUIRED.";
                    }
                }
            }
        }

        int width = 0;
        int height = 0;
        SDL_GetRendererOutputSize(renderer, &width, &height);
        render_beginner(renderer, width, height, session);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    if (launch_classic) return formfactor_classic_workbench_main(argc, argv);
    return 0;
}
