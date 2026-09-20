#include "formfactor/box3d_physics.hpp"

#include <box3d/box3d.h>

#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace formfactor::physics {
namespace {

bool finite(Vec3 value) {
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

void require_finite(Vec3 value, const char* label) {
    if (!finite(value)) {
        throw std::invalid_argument(label);
    }
}

void require_positive_extents(Vec3 half_extents) {
    require_finite(half_extents, "Box3D box extents must be finite");
    if (half_extents.x <= 0.0 || half_extents.y <= 0.0 || half_extents.z <= 0.0) {
        throw std::invalid_argument("Box3D box half extents must be greater than zero");
    }
}

float as_float(double value) {
    return static_cast<float>(value);
}

b3Pos to_position(Vec3 value) {
    return b3Pos{as_float(value.x), as_float(value.y), as_float(value.z)};
}

b3Vec3 to_vector(Vec3 value) {
    return b3Vec3{as_float(value.x), as_float(value.y), as_float(value.z)};
}

Vec3 from_position(b3Pos value) {
    return Vec3{static_cast<double>(value.x),
                static_cast<double>(value.y),
                static_cast<double>(value.z)};
}

Vec3 from_vector(b3Vec3 value) {
    return Vec3{static_cast<double>(value.x),
                static_cast<double>(value.y),
                static_cast<double>(value.z)};
}

}  // namespace

struct Box3DWorld::Impl {
    b3WorldId world{};
    std::vector<b3BodyId> bodies;

    Impl() {
        b3WorldDef world_def = b3DefaultWorldDef();
        world_def.gravity = b3Vec3{0.0F, -9.80665F, 0.0F};
        world = b3CreateWorld(&world_def);
        if (!b3World_IsValid(world)) {
            throw std::runtime_error("Box3D could not create a physics world");
        }
    }

    ~Impl() {
        if (b3World_IsValid(world)) {
            b3DestroyWorld(world);
        }
    }

    std::size_t add_box(Vec3 center,
                        Vec3 half_extents,
                        b3BodyType type,
                        double density,
                        double friction) {
        require_finite(center, "Box3D body position must be finite");
        require_positive_extents(half_extents);
        if (!std::isfinite(friction) || friction < 0.0) {
            throw std::invalid_argument("Box3D friction must be finite and non-negative");
        }
        if (!std::isfinite(density) || density < 0.0) {
            throw std::invalid_argument("Box3D density must be finite and non-negative");
        }
        if (type == b3_dynamicBody && density <= 0.0) {
            throw std::invalid_argument("A moving Box3D body needs density greater than zero");
        }

        b3BodyDef body_def = b3DefaultBodyDef();
        body_def.type = type;
        body_def.position = to_position(center);
        const b3BodyId body = b3CreateBody(world, &body_def);

        const b3BoxHull hull = b3MakeBoxHull(as_float(half_extents.x),
                                             as_float(half_extents.y),
                                             as_float(half_extents.z));
        b3ShapeDef shape_def = b3DefaultShapeDef();
        shape_def.density = as_float(density);
        shape_def.baseMaterial.friction = as_float(friction);
        b3CreateHullShape(body, &shape_def, &hull.base);

        bodies.push_back(body);
        return bodies.size() - 1U;
    }
};

Box3DWorld::Box3DWorld() : impl_(std::make_unique<Impl>()) {}
Box3DWorld::~Box3DWorld() = default;
Box3DWorld::Box3DWorld(Box3DWorld&&) noexcept = default;
Box3DWorld& Box3DWorld::operator=(Box3DWorld&&) noexcept = default;

bool Box3DWorld::valid() const noexcept {
    return impl_ != nullptr && b3World_IsValid(impl_->world);
}

void Box3DWorld::set_gravity(Vec3 gravity) {
    if (!impl_) {
        throw std::logic_error("Cannot use a moved-from Box3D world");
    }
    require_finite(gravity, "Box3D gravity must be finite");
    b3World_SetGravity(impl_->world, to_vector(gravity));
}

std::size_t Box3DWorld::add_static_box(Vec3 center,
                                       Vec3 half_extents,
                                       double friction) {
    if (!impl_) {
        throw std::logic_error("Cannot use a moved-from Box3D world");
    }
    return impl_->add_box(center, half_extents, b3_staticBody, 0.0, friction);
}

std::size_t Box3DWorld::add_dynamic_box(Vec3 center,
                                        Vec3 half_extents,
                                        double density,
                                        double friction) {
    if (!impl_) {
        throw std::logic_error("Cannot use a moved-from Box3D world");
    }
    return impl_->add_box(center, half_extents, b3_dynamicBody, density, friction);
}

void Box3DWorld::step(double seconds, int substeps) {
    if (!impl_) {
        throw std::logic_error("Cannot use a moved-from Box3D world");
    }
    if (!std::isfinite(seconds) || seconds <= 0.0) {
        throw std::invalid_argument("Box3D step time must be finite and greater than zero");
    }
    if (substeps < 1) {
        throw std::invalid_argument("Box3D substeps must be at least one");
    }
    b3World_Step(impl_->world, as_float(seconds), substeps);
}

BodyState Box3DWorld::body_state(std::size_t body_index) const {
    if (!impl_) {
        throw std::logic_error("Cannot use a moved-from Box3D world");
    }
    if (body_index >= impl_->bodies.size()) {
        throw std::out_of_range("Box3D body index is outside the FormFactor physics world");
    }

    const b3BodyId body = impl_->bodies[body_index];
    return BodyState{
        from_position(b3Body_GetPosition(body)),
        from_vector(b3Body_GetLinearVelocity(body))};
}

std::size_t Box3DWorld::body_count() const noexcept {
    return impl_ ? impl_->bodies.size() : 0U;
}

}  // namespace formfactor::physics
