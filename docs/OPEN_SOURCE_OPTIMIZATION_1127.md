# FormFactor 1.127 — Open-source optimization research

Research date: 2026-09-18.

This run started by reading FormFactor's current C++ core, SDL2 FreeLab, Godot interaction stack, validation scripts, packaging workflow, and release artifacts before changing gameplay. The current implementation was then compared with open-source EDA, procedural-electronics, editor, and PCB-viewer projects.

## References reviewed

- KiCad Packages3D source (GitLab): https://gitlab.com/kicad/libraries/kicad-packages3D-source
  - Reference for maintainable scripted and parametric electronics geometry.
- Signex (GitHub): https://github.com/alplabai/signex
  - Reference for separating schematic/PCB editing, 3D viewing, simulation, and plugin responsibilities.
- SparkBench (GitHub): https://github.com/photon-cat/sparkbench
  - Reference for drag/drop component editing, footprint rotation, routing, design-rule feedback, undo/redo, and 3D board preview.
- Ohmlet (GitHub): https://github.com/JENW1N/ohmlet
  - Reference for procedural component meshes and multiple render-quality modes in an electronics simulator.
- C-PCB (GitHub): https://github.com/vygr/C-PCB
  - C++ reference for keeping routing/solver data separate from the viewer.

GitHub, GitLab, SourceForge, Codeberg, and Bitbucket were searched for directly relevant maintainable references. Codeberg pages were not machine-readable in this session, and no stronger directly relevant reference surfaced from SourceForge or Bitbucket than the maintained projects above.

## Changes derived from the review

1. Release truth now matches source truth: the packaging job runs the current gameplay regression before publication, and stable latest-download aliases are rebuilt instead of leaving stale assets behind.
2. Editor state is safer: click placement rejects overlap and edit history is bounded.
3. Direct manipulation is faster: Delete/Backspace removes the active component and Ctrl+D duplicates it into the nearest valid grid location.
4. The PCB remains a blank text-free canvas while using a subtle grid, visible board depth, and soft rim light to improve spatial reading.
5. The scene now points to a stable current-entry script so versioned regression layers can continue without changing main.tscn every release.

No third-party source code, game assets, symbols, or 3D models were copied into FormFactor. These references informed architecture and interaction patterns only.
