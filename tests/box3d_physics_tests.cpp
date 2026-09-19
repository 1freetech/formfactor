#include "formfactor/box3d_physics.hpp"

#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>

int main() {
    using formfactor::physics::Box3DWorld;
    using formfactor::physics::Vec3;

    try {
        Box3DWorld world;
        if (!world.valid()) {
            std::cerr << "Box3D world was not valid after creation\n";
            return 1;
        }

        const std::size_t bench = world.add_static_box(
            Vec3{0.0, -0.5, 0.0}, Vec3{4.0, 0.5, 4.0});
        const std::size_t part = world.add_dynamic_box(
            Vec3{0.0, 2.0, 0.0}, Vec3{0.25, 0.25, 0.25}, 900.0);

        if (bench != 0U || part != 1U || world.body_count() != 2U) {
            std::cerr << "Box3D body indexing did not stay deterministic\n";
            return 1;
        }

        const double start_y = world.body_state(part).position.y;
        for (int frame = 0; frame < 180; ++frame) {
            world.step(1.0 / 60.0, 4);
        }

        const auto settled = world.body_state(part);
        if (!(settled.position.y < start_y)) {
            std::cerr << "Dynamic body did not fall under gravity\n";
            return 1;
        }
        if (std::abs(settled.position.y - 0.25) > 0.05) {
            std::cerr << "Dynamic body did not settle on the work surface: y="
                      << settled.position.y << '\n';
            return 1;
        }
        if (std::abs(settled.linear_velocity.y) > 0.1) {
            std::cerr << "Dynamic body did not come to rest: vy="
                      << settled.linear_velocity.y << '\n';
            return 1;
        }

        bool rejected_bad_shape = false;
        try {
            static_cast<void>(world.add_dynamic_box(
                Vec3{0.0, 1.0, 0.0}, Vec3{0.0, 0.2, 0.2}, 1000.0));
        } catch (const std::invalid_argument&) {
            rejected_bad_shape = true;
        }
        if (!rejected_bad_shape) {
            std::cerr << "Physics bridge accepted a zero-sized body\n";
            return 1;
        }

        std::cout << "FormFactor Box3D physics smoke test passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Unexpected Box3D physics error: " << error.what() << '\n';
        return 1;
    }
}
