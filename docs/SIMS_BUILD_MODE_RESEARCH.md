# The Sims Build-Mode Lessons for FormFactor

Updated: September 15, 2026

FormFactor is the game. **Workbench / FreeLab** is the current sandbox editing mode inside FormFactor.

This note records useful interaction lessons from The Sims 4 build mode without copying proprietary code, art, audio, UI assets, branding, object data, or game content.

## Useful build-mode ideas

The Sims 4 build workflow makes construction fast by giving common editing actions direct controls instead of forcing the player back through menus for every change. EA's published controls include undo/redo, object rotation, grid and precision placement, delete, and other direct build-mode commands. Community build documentation also records an Eyedropper tool that copies an existing object selection so the builder does not have to search the catalogue again.

Sources:

- EA, The Sims 4 Tips & Tricks: https://www.ea.com/games/the-sims/tips-and-tricks
- Sims Community, Getting Started With Building: https://simscommunity.info/2019/05/26/the-sims-4-getting-started-with-building/

## FormFactor translation

The important lesson is not to imitate The Sims visually. The useful lesson is **fast, forgiving direct manipulation**.

FormFactor now translates that idea into engineering-safe controls:

- **E — Pick Same Part:** focus a placed component and press E. FormFactor selects that component type so another can be placed immediately.
- **Ctrl+D — Duplicate:** makes another copy in the nearest open grid location. Wires are not copied because new electrical connections should remain explicit.
- **Shift+Arrow — Precise Nudge:** moves the focused component one grid step while preserving board-edge and overlap rules.
- **X — Disconnect:** removes wires from the focused component without deleting the component so it can be rewired.
- **Ctrl+Z / Ctrl+Y:** preserves the existing undo/redo safety path.

## Engineering-truth rule

These controls only change editing speed and usability. They do not alter component electrical meaning, validation results, compatibility rules, or engineering evidence.

A duplicated or picked component is still subject to the same FormFactor component identity and validation rules. A disconnected circuit becomes electrically disconnected until the player creates valid new connections and retests it.
