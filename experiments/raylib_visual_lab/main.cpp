#include <raylib.h>
#include <raymath.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <string>

namespace {

struct PartCard {
    const char* code;
    const char* name;
    const char* description;
    Vector3 position;
    Vector3 bounds;
};

constexpr std::array<PartCard, 6> kParts{{
    {"R1", "AXIAL RESISTOR", "Limits current and creates a controlled voltage drop.", {-2.8F, 0.55F, -1.45F}, {2.15F, 0.75F, 0.75F}},
    {"C1", "ELECTROLYTIC CAPACITOR", "Stores charge and helps smooth or filter voltage changes.", {-0.65F, 0.78F, -1.45F}, {1.0F, 1.55F, 1.0F}},
    {"D1", "5 MM LED", "Lights when current flows through it in the correct direction.", {1.30F, 0.72F, -1.45F}, {1.0F, 1.45F, 1.0F}},
    {"U1", "DIP INTEGRATED CIRCUIT", "Represents a chip that uses power and signal pins.", {-2.15F, 0.50F, 1.10F}, {2.40F, 0.80F, 1.55F}},
    {"BT1", "BATTERY STYLE SOURCE", "Supplies electrical energy to the training board.", {0.65F, 0.58F, 1.25F}, {1.75F, 1.00F, 1.30F}},
    {"J1", "PIN HEADER", "Gives power or signals a physical path into or out of the board.", {3.05F, 0.42F, 1.05F}, {1.55F, 0.75F, 1.05F}},
}};

Color PanelColor() { return Color{10, 16, 18, 245}; }
Color Neon() { return Color{57, 255, 20, 255}; }
Color Muted() { return Color{164, 181, 185, 255}; }
Color Copper() { return Color{221, 134, 42, 255}; }

void DrawBoard() {
    DrawCube({0.0F, 0.0F, 0.0F}, 9.8F, 0.28F, 6.2F, Color{24, 91, 64, 255});
    DrawCubeWires({0.0F, 0.0F, 0.0F}, 9.8F, 0.28F, 6.2F, Neon());

    const float y = 0.16F;
    const Color trace{184, 105, 28, 255};
    for (int i = -4; i <= 4; ++i) {
        const float x = static_cast<float>(i);
        DrawCube({x, y, 0.0F}, 0.035F, 0.02F, 5.5F, trace);
    }
    for (int i = -2; i <= 2; ++i) {
        const float z = static_cast<float>(i);
        DrawCube({0.0F, y, z}, 8.9F, 0.02F, 0.035F, trace);
    }

    for (int x = -4; x <= 4; ++x) {
        for (int z = -2; z <= 2; ++z) {
            DrawCylinder({static_cast<float>(x), 0.22F, static_cast<float>(z)}, 0.055F, 0.055F, 0.03F, 10, Color{230, 176, 54, 255});
        }
    }
}

void DrawResistor(const Vector3& p) {
    const Vector3 a{p.x - 0.95F, p.y, p.z};
    const Vector3 b{p.x + 0.95F, p.y, p.z};
    DrawCylinderEx(a, {p.x - 0.58F, p.y, p.z}, 0.045F, 0.045F, 10, LIGHTGRAY);
    DrawCylinderEx({p.x + 0.58F, p.y, p.z}, b, 0.045F, 0.045F, 10, LIGHTGRAY);
    DrawCylinderEx({p.x - 0.58F, p.y, p.z}, {p.x + 0.58F, p.y, p.z}, 0.23F, 0.23F, 18, Color{210, 171, 103, 255});

    const std::array<Color, 4> bands{BROWN, RED, BLACK, GOLD};
    const std::array<float, 4> offsets{-0.31F, -0.09F, 0.14F, 0.37F};
    for (std::size_t i = 0; i < bands.size(); ++i) {
        DrawCylinderEx({p.x + offsets[i] - 0.027F, p.y, p.z}, {p.x + offsets[i] + 0.027F, p.y, p.z}, 0.245F, 0.245F, 18, bands[i]);
    }
}

void DrawCapacitor(const Vector3& p) {
    DrawCylinder({p.x, p.y - 0.48F, p.z}, 0.29F, 0.29F, 0.95F, 24, Color{42, 96, 169, 255});
    DrawCylinderWires({p.x, p.y - 0.48F, p.z}, 0.29F, 0.29F, 0.95F, 24, Color{210, 220, 229, 255});
    DrawCube({p.x + 0.22F, p.y, p.z}, 0.055F, 0.82F, 0.52F, Color{226, 231, 235, 255});
    DrawCylinderEx({p.x - 0.11F, 0.20F, p.z}, {p.x - 0.11F, p.y - 0.44F, p.z}, 0.025F, 0.025F, 8, LIGHTGRAY);
    DrawCylinderEx({p.x + 0.11F, 0.20F, p.z}, {p.x + 0.11F, p.y - 0.44F, p.z}, 0.025F, 0.025F, 8, LIGHTGRAY);
}

void DrawLed(const Vector3& p) {
    DrawCylinderEx({p.x - 0.11F, 0.20F, p.z}, {p.x - 0.11F, p.y - 0.18F, p.z}, 0.025F, 0.025F, 8, LIGHTGRAY);
    DrawCylinderEx({p.x + 0.11F, 0.20F, p.z}, {p.x + 0.11F, p.y - 0.11F, p.z}, 0.025F, 0.025F, 8, LIGHTGRAY);
    DrawCylinder({p.x, p.y - 0.25F, p.z}, 0.28F, 0.28F, 0.42F, 24, Color{198, 32, 47, 220});
    DrawSphere({p.x, p.y + 0.16F, p.z}, 0.28F, Color{233, 45, 60, 220});
    DrawSphereWires({p.x, p.y + 0.16F, p.z}, 0.29F, 8, 16, Color{255, 125, 135, 255});
}

void DrawChip(const Vector3& p) {
    DrawCube(p, 1.70F, 0.36F, 1.02F, Color{29, 32, 36, 255});
    DrawCubeWires(p, 1.70F, 0.36F, 1.02F, Color{117, 124, 130, 255});
    for (int i = 0; i < 6; ++i) {
        const float x = p.x - 0.68F + static_cast<float>(i) * 0.27F;
        DrawCube({x, p.y - 0.04F, p.z - 0.66F}, 0.10F, 0.08F, 0.34F, LIGHTGRAY);
        DrawCube({x, p.y - 0.04F, p.z + 0.66F}, 0.10F, 0.08F, 0.34F, LIGHTGRAY);
    }
    DrawSphere({p.x - 0.64F, p.y + 0.20F, p.z}, 0.07F, Color{126, 132, 138, 255});
}

void DrawBattery(const Vector3& p) {
    DrawCube(p, 1.45F, 0.72F, 0.95F, Color{48, 51, 56, 255});
    DrawCubeWires(p, 1.45F, 0.72F, 0.95F, Color{147, 154, 161, 255});
    DrawCylinder({p.x - 0.33F, p.y + 0.43F, p.z}, 0.12F, 0.12F, 0.18F, 14, Color{221, 56, 56, 255});
    DrawCylinder({p.x + 0.33F, p.y + 0.43F, p.z}, 0.12F, 0.12F, 0.18F, 14, Color{40, 44, 47, 255});
}

void DrawHeader(const Vector3& p) {
    DrawCube(p, 1.38F, 0.22F, 0.54F, Color{28, 31, 33, 255});
    for (int i = 0; i < 6; ++i) {
        const float x = p.x - 0.56F + static_cast<float>(i) * 0.225F;
        DrawCube({x, p.y + 0.31F, p.z}, 0.07F, 0.66F, 0.07F, Color{218, 177, 58, 255});
    }
}

void DrawPart3D(std::size_t index) {
    const Vector3 p = kParts[index].position;
    switch (index) {
        case 0: DrawResistor(p); break;
        case 1: DrawCapacitor(p); break;
        case 2: DrawLed(p); break;
        case 3: DrawChip(p); break;
        case 4: DrawBattery(p); break;
        case 5: DrawHeader(p); break;
        default: break;
    }
}

void DrawSelection(std::size_t selected) {
    const PartCard& part = kParts[selected];
    DrawCubeWires(part.position, part.bounds.x, part.bounds.y, part.bounds.z, Color{255, 224, 80, 255});
    DrawSphereWires({part.position.x, part.position.y + part.bounds.y * 0.70F, part.position.z}, 0.16F, 6, 12, Neon());
}

void DrawSchematicSymbol(std::size_t selected, Rectangle box) {
    const float cy = box.y + box.height * 0.52F;
    const float left = box.x + 28.0F;
    const float right = box.x + box.width - 28.0F;
    const float mid = (left + right) * 0.5F;
    const Color c = Color{224, 233, 235, 255};

    if (selected == 0) {
        DrawLineEx({left, cy}, {left + 24, cy}, 2.0F, c);
        const std::array<Vector2, 8> pts{{
            {left + 24, cy}, {left + 38, cy - 12}, {left + 52, cy + 12}, {left + 66, cy - 12},
            {left + 80, cy + 12}, {left + 94, cy - 12}, {left + 108, cy}, {right, cy}
        }};
        for (std::size_t i = 1; i < pts.size(); ++i) DrawLineEx(pts[i - 1], pts[i], 2.0F, c);
    } else if (selected == 1) {
        DrawLineEx({left, cy}, {mid - 12, cy}, 2.0F, c);
        DrawLineEx({mid - 12, cy - 24}, {mid - 12, cy + 24}, 3.0F, c);
        DrawLineEx({mid + 12, cy - 24}, {mid + 12, cy + 24}, 3.0F, c);
        DrawLineEx({mid + 12, cy}, {right, cy}, 2.0F, c);
        DrawText("+", static_cast<int>(mid - 28), static_cast<int>(cy - 48), 22, Neon());
    } else if (selected == 2) {
        DrawLineEx({left, cy}, {mid - 24, cy}, 2.0F, c);
        DrawTriangle({mid - 24, cy - 22}, {mid - 24, cy + 22}, {mid + 8, cy}, c);
        DrawLineEx({mid + 10, cy - 24}, {mid + 10, cy + 24}, 3.0F, c);
        DrawLineEx({mid + 10, cy}, {right, cy}, 2.0F, c);
        DrawLineEx({mid + 22, cy - 22}, {mid + 42, cy - 42}, 2.0F, Neon());
        DrawLineEx({mid + 34, cy - 8}, {mid + 54, cy - 28}, 2.0F, Neon());
    } else if (selected == 3) {
        DrawRectangleLinesEx({mid - 50, cy - 38, 100, 76}, 2.0F, c);
        for (int i = 0; i < 4; ++i) {
            const float y = cy - 27.0F + static_cast<float>(i) * 18.0F;
            DrawLineEx({mid - 72, y}, {mid - 50, y}, 2.0F, c);
            DrawLineEx({mid + 50, y}, {mid + 72, y}, 2.0F, c);
        }
        DrawText("IC", static_cast<int>(mid - 14), static_cast<int>(cy - 11), 20, c);
    } else if (selected == 4) {
        DrawLineEx({left, cy}, {mid - 22, cy}, 2.0F, c);
        DrawLineEx({mid - 22, cy - 30}, {mid - 22, cy + 30}, 3.0F, c);
        DrawLineEx({mid + 8, cy - 18}, {mid + 8, cy + 18}, 3.0F, c);
        DrawLineEx({mid + 8, cy}, {right, cy}, 2.0F, c);
        DrawText("+", static_cast<int>(mid - 42), static_cast<int>(cy - 50), 20, Neon());
        DrawText("-", static_cast<int>(mid + 19), static_cast<int>(cy - 48), 20, Muted());
    } else {
        for (int i = 0; i < 6; ++i) {
            const float y = box.y + 28.0F + static_cast<float>(i) * 14.0F;
            DrawCircleV({mid - 20.0F, y}, 4.0F, c);
            DrawLineEx({mid - 16.0F, y}, {mid + 34.0F, y}, 2.0F, c);
        }
    }
}

void DrawInterface(std::size_t selected, const std::array<bool, 6>& seen) {
    const int screenW = GetScreenWidth();
    const int screenH = GetScreenHeight();

    DrawRectangle(0, 0, screenW, 66, Color{11, 17, 19, 245});
    DrawRectangle(0, 63, screenW, 3, Neon());
    DrawText("FORMFACTOR - OPEN SOURCE 3D VISUAL LAB", 20, 17, 24, Neon());
    DrawText("Experimental branch: presentation + camera + component fidelity", 20, 43, 15, Muted());

    const Rectangle list{18.0F, 86.0F, 270.0F, static_cast<float>(screenH - 108)};
    DrawRectangleRec(list, PanelColor());
    DrawRectangleLinesEx(list, 1.0F, Color{63, 89, 92, 255});
    DrawText("PART INSPECTION", 36, 104, 18, Neon());
    DrawText("Click a card or press TAB", 36, 128, 14, Muted());

    for (std::size_t i = 0; i < kParts.size(); ++i) {
        const Rectangle row{34.0F, 160.0F + static_cast<float>(i) * 68.0F, 238.0F, 54.0F};
        const bool active = i == selected;
        DrawRectangleRec(row, active ? Color{31, 62, 39, 255} : Color{20, 27, 29, 255});
        DrawRectangleLinesEx(row, active ? 2.0F : 1.0F, active ? Neon() : Color{64, 76, 79, 255});
        DrawText(kParts[i].code, static_cast<int>(row.x + 10), static_cast<int>(row.y + 8), 18, active ? Neon() : RAYWHITE);
        DrawText(kParts[i].name, static_cast<int>(row.x + 48), static_cast<int>(row.y + 9), 14, RAYWHITE);
        DrawText(seen[i] ? "INSPECTED" : "NEW", static_cast<int>(row.x + 48), static_cast<int>(row.y + 31), 12, seen[i] ? Neon() : Color{255, 205, 86, 255});
    }

    int seenCount = 0;
    for (bool value : seen) if (value) ++seenCount;
    DrawText(TextFormat("INSPECTION GOAL: %d / 6", seenCount), 36, screenH - 84, 17, seenCount == 6 ? Neon() : RAYWHITE);
    DrawText(seenCount == 6 ? "COMPONENT TOUR COMPLETE" : "Inspect all six parts", 36, screenH - 59, 14, seenCount == 6 ? Neon() : Muted());

    const float panelW = 330.0F;
    const Rectangle inspector{static_cast<float>(screenW) - panelW - 18.0F, 86.0F, panelW, static_cast<float>(screenH - 108)};
    DrawRectangleRec(inspector, PanelColor());
    DrawRectangleLinesEx(inspector, 1.0F, Color{63, 89, 92, 255});
    DrawText("PHYSICAL + SCHEMATIC", static_cast<int>(inspector.x + 18), 104, 18, Neon());
    DrawText(kParts[selected].code, static_cast<int>(inspector.x + 18), 142, 27, RAYWHITE);
    DrawText(kParts[selected].name, static_cast<int>(inspector.x + 72), 148, 15, RAYWHITE);

    const Rectangle symbolBox{inspector.x + 18.0F, 184.0F, inspector.width - 36.0F, 132.0F};
    DrawRectangleRec(symbolBox, Color{13, 20, 22, 255});
    DrawRectangleLinesEx(symbolBox, 1.0F, Color{75, 96, 99, 255});
    DrawSchematicSymbol(selected, symbolBox);

    DrawText("ONE-LINE CODEX", static_cast<int>(inspector.x + 18), 340, 16, Neon());
    DrawText(kParts[selected].description, static_cast<int>(inspector.x + 18), 370, 14, RAYWHITE);

    DrawText("CAMERA", static_cast<int>(inspector.x + 18), 430, 16, Neon());
    DrawText("Right drag: orbit", static_cast<int>(inspector.x + 18), 458, 14, Muted());
    DrawText("Mouse wheel: zoom", static_cast<int>(inspector.x + 18), 480, 14, Muted());
    DrawText("WASD: move target", static_cast<int>(inspector.x + 18), 502, 14, Muted());
    DrawText("R: reset camera", static_cast<int>(inspector.x + 18), 524, 14, Muted());
    DrawText("TAB / arrows: inspect", static_cast<int>(inspector.x + 18), 546, 14, Muted());

    DrawText("WHY THIS LAB EXISTS", static_cast<int>(inspector.x + 18), 596, 16, Neon());
    DrawText("Test a real 3D camera, lighting", static_cast<int>(inspector.x + 18), 624, 14, Muted());
    DrawText("and exact-looking package shapes", static_cast<int>(inspector.x + 18), 646, 14, Muted());
    DrawText("before changing the main game.", static_cast<int>(inspector.x + 18), 668, 14, Muted());

    DrawText("ESC exits", screenW / 2 - 35, screenH - 31, 14, Muted());
}

}  // namespace

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1440, 860, "FormFactor Open Source 3D Visual Lab");
    SetTargetFPS(60);

    Camera3D camera{};
    camera.target = {0.0F, 0.0F, 0.0F};
    camera.up = {0.0F, 1.0F, 0.0F};
    camera.fovy = 48.0F;
    camera.projection = CAMERA_PERSPECTIVE;

    float yaw = -0.75F;
    float pitch = 0.72F;
    float radius = 11.5F;
    std::size_t selected = 0;
    std::array<bool, 6> seen{};
    seen[selected] = true;

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        const Vector2 mouse = GetMousePosition();

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            const Vector2 delta = GetMouseDelta();
            yaw -= delta.x * 0.006F;
            pitch = Clamp(pitch - delta.y * 0.006F, 0.16F, 1.42F);
        }
        radius = Clamp(radius - GetMouseWheelMove() * 0.75F, 6.0F, 18.0F);

        const float move = 3.0F * dt;
        if (IsKeyDown(KEY_W)) camera.target.z -= move;
        if (IsKeyDown(KEY_S)) camera.target.z += move;
        if (IsKeyDown(KEY_A)) camera.target.x -= move;
        if (IsKeyDown(KEY_D)) camera.target.x += move;
        if (IsKeyPressed(KEY_R)) {
            camera.target = {0.0F, 0.0F, 0.0F};
            yaw = -0.75F;
            pitch = 0.72F;
            radius = 11.5F;
        }

        const bool reverseTab = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_DOWN)) {
            selected = reverseTab ? (selected + kParts.size() - 1) % kParts.size() : (selected + 1) % kParts.size();
            seen[selected] = true;
        }
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_UP)) {
            selected = (selected + kParts.size() - 1) % kParts.size();
            seen[selected] = true;
        }

        for (std::size_t i = 0; i < kParts.size(); ++i) {
            const Rectangle row{34.0F, 160.0F + static_cast<float>(i) * 68.0F, 238.0F, 54.0F};
            if (CheckCollisionPointRec(mouse, row) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                selected = i;
                seen[selected] = true;
            }
        }

        const float cp = std::cos(pitch);
        camera.position = {
            camera.target.x + radius * cp * std::sin(yaw),
            camera.target.y + radius * std::sin(pitch),
            camera.target.z + radius * cp * std::cos(yaw)
        };

        BeginDrawing();
        ClearBackground(Color{7, 10, 12, 255});

        BeginMode3D(camera);
        DrawPlane({0.0F, -0.20F, 0.0F}, {40.0F, 40.0F}, Color{18, 23, 25, 255});
        DrawGrid(24, 1.0F);
        DrawBoard();
        for (std::size_t i = 0; i < kParts.size(); ++i) DrawPart3D(i);
        DrawSelection(selected);
        EndMode3D();

        DrawInterface(selected, seen);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
