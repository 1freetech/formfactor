# Box3D Physics in FormFactor — Update 1.06

## Why Box3D is useful

Box3D gives FormFactor a real 3D physics engine instead of making the game fake every physical interaction by hand. It can handle gravity, collisions, moving rigid parts, joints, sensors, ray casts, and large groups of physical bodies.

That is useful for the game side of FormFactor. A component can fall onto a bench, a connector can hit another object, a tool can collide with a board, and later a cable or mechanical assembly can react to movement in a believable way.

## What Box3D is not allowed to decide

Box3D does not decide whether a PCB works.

The FormFactor engineering core still decides circuit rules, component data validity, electrical simulation evidence, layout rules, power integrity requirements, and manufacturing gates. Physics can show a physical event. It cannot turn a bad circuit into a good circuit.

This keeps the project rule simple:

**Box3D handles physical motion. FormFactor handles engineering truth.**

## Current implementation

Update 1.06 adds a small FormFactor-owned wrapper around Box3D. The wrapper can:

- create a physics world;
- set gravity;
- create static boxes;
- create moving boxes with density and friction;
- advance the world with fixed substeps;
- read a body's position and speed; and
- reject invalid sizes, density, time steps, and body indexes.

A CTest smoke test creates a solid work surface and drops a component-sized body onto it. The test passes only if gravity moves the body down and collision stops it on the surface.

## Safe build switch

The normal FormFactor build does not need Box3D yet. Turn the new backend on with:

```bash
cmake -S . -B build-box3d -DFORMFACTOR_ENABLE_BOX3D_PHYSICS=ON
cmake --build build-box3d --parallel 2
ctest --test-dir build-box3d -R box3d_physics_smoke --output-on-failure
```

The integration is pinned to Box3D `v0.1.0` so a future upstream change cannot silently change FormFactor's physics build.

## Next useful steps

The next stage should connect the backend to the visible 3D lab in small pieces: component drop and settle animation, collision-aware placement, physical picking and ray casts, connector and tool interactions, then joints for mechanical assemblies. Each step should keep a simple fallback path until it is tested on Windows and Linux.

Box3D is a physics library, not a renderer. SDL still draws the current FreeLab prototype. A future renderer can change independently without throwing away the Box3D physics layer.

## License

Box3D is MIT licensed. FormFactor should keep the Box3D license notice whenever Box3D source or binary distributions require it.
