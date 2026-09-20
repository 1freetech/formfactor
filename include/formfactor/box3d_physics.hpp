#pragma once

#include <cstddef>
#include <memory>

namespace formfactor::physics {

struct Vec3 {
    double x{0.0};
    double y{0.0};
    double z{0.0};
};

struct BodyState {
    Vec3 position{};
    Vec3 linear_velocity{};
};

class Box3DWorld {
public:
    Box3DWorld();
    ~Box3DWorld();

    Box3DWorld(const Box3DWorld&) = delete;
    Box3DWorld& operator=(const Box3DWorld&) = delete;
    Box3DWorld(Box3DWorld&&) noexcept;
    Box3DWorld& operator=(Box3DWorld&&) noexcept;

    [[nodiscard]] bool valid() const noexcept;
    void set_gravity(Vec3 gravity);

    [[nodiscard]] std::size_t add_static_box(
        Vec3 center,
        Vec3 half_extents,
        double friction = 0.6);

    [[nodiscard]] std::size_t add_dynamic_box(
        Vec3 center,
        Vec3 half_extents,
        double density,
        double friction = 0.6);

    void step(double seconds, int substeps = 4);

    [[nodiscard]] BodyState body_state(std::size_t body_index) const;
    [[nodiscard]] std::size_t body_count() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace formfactor::physics
