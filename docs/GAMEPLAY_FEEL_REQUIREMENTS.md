# FormFactor Gameplay Feel Requirements

Updated: September 15, 2026

Status: gameplay-first design contract for the production frontend. The engineering core remains authoritative. These requirements define how FormFactor should feel to play while keeping the simulation realistic.

## Fresh all-time gameplay scan

There is no objective list of the ten "most fun" games ever. A fresh comparison of current expert and aggregate greatest-game lists repeatedly surfaces the same design families. For FormFactor, the most useful ten-game gameplay reference set is:

1. **Tetris** - instant rules, immediate feedback, almost no setup, fast restart, mastery through repetition.
2. **Super Mario 64** - direct responsive movement, readable spaces, controls that feel good before any reward system is added.
3. **Super Mario World** - simple inputs, tight cause-and-effect, short challenges, strong pacing.
4. **The Legend of Zelda: Ocarina of Time** - contextual actions, clear targeting, exploration that does not require constant menu use.
5. **Doom** - speed, direct input, readable threats, constant forward momentum.
6. **Resident Evil 4** - excellent pacing, meaningful resource choices, tension followed by release, strong action readability.
7. **Half-Life 2** - systems taught through play, physical cause-and-effect, varied situations built from a small set of understandable rules.
8. **Portal 2** - learn by doing, very clear feedback, fast experimentation, humor and presentation that reduce frustration.
9. **Minecraft** - freedom, building, discovery, simple parts combining into deep emergent systems.
10. **The Legend of Zelda: Breath of the Wild** - systemic experimentation, player freedom, multiple valid solutions, curiosity rewarded.

Other recurring references such as Dark Souls, Super Metroid, Street Fighter II, Halo, and Grand Theft Auto reinforce the same larger lessons: controls must feel trustworthy, failure must be understandable, and mastery must come from learning the system rather than fighting the interface.

### Research starting points

- The Greatest Games aggregate: https://thegreatest.games/
- GQ expert ranking, updated July 2026: https://www.gq-magazine.co.uk/article/best-video-games-all-time
- Slant Magazine's all-time list: https://www.slantmagazine.com/games/the-100-best-video-games-of-all-time/
- EDGE greatest-games ranking archive coverage: https://nintendoeverything.com/edge-ranks-the-100-greatest-video-games/

These references are used for high-level design ideas only. FormFactor does not copy proprietary code, art, audio, levels, characters, branding, UI expression, or other protected content.

## Gameplay-first mandate

FormFactor should feel like a game immediately, while the engineering core stays realistic underneath.

The player should spend time **building, testing, diagnosing, repairing, and learning**, not fighting menus.

The production frontend should optimize for:

- direct control;
- fast readable response;
- low friction;
- clear cause and effect;
- fast recovery from mistakes;
- satisfying repeated actions;
- multiple valid solutions;
- gradual mastery;
- realistic results;
- optional depth instead of forced complexity.

## Core fun loop

The main loop remains:

**Place -> Connect -> Power -> Test -> Diagnose -> Repair -> Validate**

Each step must feel good on its own.

### Place

- Show a placement preview before committing.
- Snap cleanly to useful board/grid positions while allowing the user to override snap where appropriate.
- Show orientation before placement.
- Make invalid placement obvious before the click when the underlying rule is already known.
- Placement should feel immediate. Do not make a normal part placement wait on unrelated deep analysis.

### Connect

- Highlight valid connection targets when a connection starts.
- Show a live wire/trace preview before completion.
- Make cancel/back obvious and immediate.
- Never silently create a connection to the wrong target.
- Common wiring should take the fewest practical actions.

### Power

- Power-up should feel important and readable.
- Show what rails or subsystems changed state.
- Never use dramatic animation to hide a failed or unknown result.

### Test

- One obvious action starts the relevant test.
- Give immediate acknowledgement that the test started.
- Longer checks show `running`, what is being checked, and what remains pending.
- Keep the player in the project while the result arrives.

### Diagnose

- Measurements should be easy to place and read.
- The UI should preserve the selected component, net, fault, or measurement target across views.
- Good diagnostic actions may be acknowledged without revealing the answer.
- Failure evidence should point the player toward the real problem area.

### Repair

- Make replacement, rotation, reconnect, value change, and reroute actions easy to reach.
- Preserve useful context after a repair instead of resetting the workspace.
- Undo/redo must become a normal editing path in the production workbench.

### Validate

- Final validation should be clear and satisfying.
- Pass, fail, unknown, and blocked states must be visually and textually distinct.
- A pass must come only from implemented engineering gates.

## Control-feel requirements

### Immediate response

For local UI actions such as hover, focus, selecting a part, opening a category, starting a connection, moving a placement preview, cancelling an action, or changing a tool, visual response should appear immediately from the player's point of view.

Heavy engineering work may take longer, but the interface must acknowledge the input immediately and show a running state.

### One action should do one understandable thing

Avoid controls where the same click unexpectedly places, connects, opens a menu, and changes selection.

Primary actions need stable meanings.

### Safe cancel

`Esc` / Back should first cancel the current temporary action or close the current overlay before exiting the game.

Starting a wire, measurement, placement, or modal operation must always have an obvious cancel path.

### Undo and redo

Production editing should support undo/redo for reversible authoring actions such as:

- placement;
- movement;
- rotation;
- deletion;
- connection/routing edits;
- editable parameter changes.

Undo never rewrites authoritative history invisibly. Saved/replayed engineering evidence must retain clear version/state identity.

### Fast selection

Support:

- mouse/touch-style direct selection where applicable;
- keyboard navigation;
- controller navigation;
- searchable component library;
- quick slots/favorites;
- Pick Same Part / eyedropper behavior;
- recent parts.

### Predict before commit

Where the result can be known cheaply before an edit commits, show a preview:

- placement collision;
- orientation;
- target pin/net;
- trace path;
- footprint outline;
- known rule warning.

Do not pretend to predict solver results that have not actually been calculated.

## Realism without friction

Realism belongs in the engineering model and evidence, not in unnecessary control difficulty.

A beginner should not need to memorize hidden keyboard commands, datasheet field names, or simulator internals just to place a resistor.

Use progressive disclosure:

- first show the part name, symbol, package, value, and important limits;
- show deeper pinout/model/provenance information in Details;
- expose advanced settings when the task needs them;
- never hide required safety or validation information.

Assistance may change hints and presentation. It may not weaken engineering truth.

## What makes repeated actions satisfying

Repeated engineering actions need small but useful confirmation:

- visible snap when a part lands correctly;
- clear highlight when a valid target is selected;
- short connection confirmation;
- readable instrument response;
- compact success/failure transition after a test;
- optional subtle audio and haptic feedback;
- before/after comparison after a repair;
- milestone feedback after meaningful validated progress.

Effects should be short and restrained. They must never cover traces, values, warnings, measurements, or symbols.

## Failure and retry

Failure is part of the fun only when it feels fair.

A failed action or test should answer:

1. What failed?
2. Where did it fail?
3. What evidence do we have?
4. What can I try next?

The player should not need to rebuild unrelated work after a normal mistake.

Engineering Contracts should use reproducible checkpoints. FreeLab should support rapid editing and retesting.

## Challenge pacing

Use short wins early and combine systems later.

Suggested progression:

1. place one component;
2. connect a tiny valid circuit;
3. power and measure it;
4. diagnose one obvious fault;
5. repair and revalidate;
6. combine several components;
7. introduce ambiguous symptoms;
8. add layout, power, signal, thermal, firmware, and manufacturing constraints only as the player is ready for them.

The game should not mistake more menus or more simultaneous warnings for higher difficulty.

## Freedom and multiple solutions

Borrow the systemic freedom seen in Minecraft, Breath of the Wild, engineering sandboxes, and immersive sims:

- FreeLab stays unrestricted within the available engineering model.
- Contracts define goals and constraints rather than one cosmetic answer.
- Any design that passes the authoritative requirements is accepted.
- Alternate diagnostic paths are allowed when they produce valid evidence.

## Prototype friction found today

The current SDL2 FreeLab smoke-test prototype already demonstrates placement, connection, validation, keyboard focus, physical parts, and schematic mirroring, but it is not the target production game feel.

Current prototype friction includes:

- help opens as a large overlay instead of letting the player begin immediately;
- the palette is a fixed six-part list;
- placement has no production-quality ghost/preview workflow;
- there is no general undo/redo path;
- keyboard navigation is stronger than keyboard placement/editing;
- connection behavior is still training-prototype logic rather than real pin/net workflow;
- `Esc` exits instead of first behaving as a normal cancel/back control;
- the visual feedback is useful but not yet production-quality game feel.

These are not reasons to overbuild the temporary SDL2 renderer. They are migration requirements for the SDL3 + bgfx + Dear ImGui production frontend.

## Production gameplay priorities

After build/test reliability and the authoritative core contracts, frontend work should prioritize this order:

1. **Direct responsive controls** - select, place, move, rotate, connect, cancel, delete, test.
2. **Placement and connection previews** - ghost parts, snapping, target highlights, trace/wire previews.
3. **Undo/redo and safe cancel** - mistakes should be cheap to correct.
4. **Fast component access** - search, categories, favorites, recent, quick slots, Pick Same Part.
5. **Readable test feedback** - running/pass/fail/unknown with evidence.
6. **Smooth minimum loop** - complete Place -> Connect -> Power -> Test -> Diagnose -> Repair -> Validate without external documentation.
7. **Practice and contracts** - focused drills and staged jobs using the same core state.
8. **Polish** - sound, haptics, motion, camera, transitions, milestone feedback.
9. **High-fidelity 3D** - only after the core 2D gameplay loop already feels good.

## Gameplay acceptance tests

The production frontend should eventually satisfy all of these:

- A first-time player can begin placing parts without dismissing a long mandatory tutorial.
- A normal local control produces visible feedback immediately.
- The player can always tell what tool/action is currently active.
- Temporary placement, connection, and measurement actions can be cancelled safely.
- Editing supports undo/redo for reversible authoring actions.
- A player can complete the minimum playable loop without external documentation.
- The same loop is reachable by mouse, keyboard-only, and gamepad-only controls.
- Common component selection does not require scrolling through the full catalogue.
- The player can retry a failed test without rebuilding unrelated work.
- Failure identifies the affected object or rule when the core has that evidence.
- Multiple engineering-valid solutions remain accepted.
- Assistance settings never change engineering pass/fail truth.
- Presentation effects can be reduced or disabled without removing required information.
- 2D Lite and full presentation use the same engineering state and validation gates.

## Core rule

**FormFactor should be easy to control, satisfying to manipulate, quick to retry, and difficult for engineering reasons.**

The interface should never be the hardest part of the engineering problem.
