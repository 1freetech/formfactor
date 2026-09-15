# NBA 2K, Spore, Space Engineers, and Extreme-Scale World Research

Status: design reference only. FormFactor adopts general product, simulation, creator, progression, streaming, and navigation ideas. It does not copy proprietary code, art, audio, branding, characters, menus, animations, maps, or protected presentation from these games.

## Why these references matter

This research fills four gaps in FormFactor's game design:

- **NBA 2K**: planning a build before committing to it, showing strengths and limits clearly, and giving long-term progression strong presentation.
- **Spore**: direct modular creation, parts that visibly change behavior, and carrying one creation through several levels of complexity.
- **Space Engineers**: engineering-first sandbox construction, blueprints, survival constraints, destructible systems, automation, and community-created scenarios.
- **Extreme-scale games**: making very large worlds usable through procedural generation, streaming, chunking, level of detail, indexing, search, and navigation.

The goal is not to make FormFactor into a basketball game, creature game, or space game. The goal is to borrow the best system-design lessons for an engineering simulator.

## NBA 2K lessons

### Build planning before commitment

NBA 2K26's MyPLAYER Builder gives players more information about a build before they finish it. Its scouting-style information, requirements, and build-by-target tools are useful references for FormFactor.

FormFactor translation:

- Add a **Design Scouting Report** before a player commits to a board or subsystem.
- Show known strengths, limitations, missing evidence, and blocked requirements.
- Let the player choose a target such as low cost, small area, low power, repairability, or a specific validated electrical requirement.
- Show the minimum known requirements for that target when authoritative data exists.
- Never promise that a build will work until the engineering core actually validates it.

A FormFactor report might say:

- `Power target: incomplete - regulator efficiency data missing.`
- `Trace-current requirement: supported - sourced limit available.`
- `3D model: visual-only - not authoritative for clearance.`
- `Manufacturing export: blocked - two required validation gates are still unknown.`

### Visible tradeoffs

A good build system makes tradeoffs understandable. FormFactor should make it obvious when improving one goal may hurt another.

Examples:

- lower cost versus lower tolerance;
- smaller area versus thermal margin;
- thicker copper versus manufacturing cost;
- easier repair versus smaller package size;
- faster simulation versus higher-fidelity analysis.

The UI may explain tradeoffs. It must not invent a preferred engineering answer.

### Fast, smarter, and full analysis profiles

NBA 2K26 exposes different simulation preferences. The FormFactor equivalent should be explicit analysis profiles where supported:

- **Preview**: fast, clearly labelled, no claim of final validation.
- **Standard**: normal interactive analysis using implemented validated methods.
- **Full**: highest supported validated analysis for final evidence.

A faster profile may do less work. It may never silently turn an unknown or failed result into a pass.

### Milestones and project history

NBA 2K uses strong presentation around career and franchise milestones. FormFactor can use restrained milestone presentation for real engineering progress:

- first valid powered circuit;
- first successful diagnosis;
- first completed board;
- first validated repair;
- first fabrication-ready project;
- completion of a training module.

Milestones are presentation only. They are earned from recorded evidence.

## Spore lessons

### Direct modular creation

Spore's strongest design idea is an editor where users assemble a complex creation from understandable parts and immediately see the result.

FormFactor translation:

- component placement should feel direct and tactile;
- values, packages, orientation, pins, and connections should be visible close to the object being edited;
- compatible actions should be easy to discover;
- the player should be able to rotate, replace, reconnect, and inspect without leaving the main workbench repeatedly.

### Parts should change behavior, not only appearance

Spore's parts can alter what a creation can do. FormFactor must go much further because engineering properties are real constraints.

A selected component should affect the project through its authoritative data:

- electrical limits;
- package and footprint;
- pins;
- simulation model;
- thermal properties;
- manufacturing constraints;
- provenance and verification state.

A visual model alone cannot create electrical behavior.

### Grow one creation through stages

Spore carries a creation through different scales. FormFactor can do the same with one engineering project:

**Component -> Circuit -> PCB -> Powered System -> Tested System -> Repaired/Optimized System -> Manufacturing Package**

The project remains the same project as complexity grows. The player should not feel like they entered an unrelated mini-game.

### Design timeline

Spore records a history of important choices. FormFactor should keep a **Project Evolution Timeline** containing reproducible milestones such as:

- component added or replaced;
- net changed;
- layout revision;
- simulation run;
- fault injected;
- measurement taken;
- repair performed;
- validation passed, failed, or became stale.

The timeline should support replay and debugging, not merely decoration.

### Procedural visuals must stay honest

Spore demonstrates powerful procedural geometry and animation. FormFactor may later generate presentation geometry procedurally from known package dimensions, but generated visuals must stay labelled appropriately.

- sourced authoritative geometry can become verified physical data;
- generated approximate geometry remains partial or visual-only;
- procedural appearance never fills missing electrical or mechanical facts.

## Space Engineers lessons

Space Engineers is especially relevant because engineering, building, testing, damage, repair, scripting, and sandbox play are its core experience.

### Creative and constrained modes use the same physics

Space Engineers separates Creative and Survival while keeping the same basic world rules.

FormFactor translation:

- **FreeLab**: unrestricted parts/tools for experimentation.
- **Engineering Contract**: explicit budget, inventory, time, documentation, or tool constraints.
- **Practice Lab**: focused training constraints.

The engineering validators stay the same in all modes.

### Blueprint system

Space Engineers lets players save and reuse designs as blueprints.

FormFactor should eventually support **Engineering Blueprints**:

- save a validated or partially validated subsystem;
- preserve exact component identities and versions;
- preserve dependencies and provenance;
- clearly show which validation evidence is reusable and which became stale;
- insert the blueprint into a new project without pretending old project-specific results still apply.

Blueprints can power tutorials, community examples, reusable power sections, test fixtures, and known-good training circuits.

### Build, damage, diagnose, repair

Space Engineers makes construction and destruction part of the same system. FormFactor should treat repair as a first-class state of the project.

Future repair scenarios may include explicit faults such as:

- open trace;
- shorted net;
- reversed component;
- failed component record;
- damaged connector;
- incorrect replacement part;
- missing power rail;
- wrong firmware or configuration where supported.

Every injected fault must be reproducible and recorded. Repair success requires re-validation.

### Automation and scripting

Space Engineers uses programmable blocks and creator scripting. This supports FormFactor's existing future scripting direction, but FormFactor must be stricter.

A future script system may:

- sequence training events;
- control presentation;
- read approved project state;
- request approved tools/actions;
- create objectives and checkpoints.

It may not directly forge measurements, validation results, provenance, or safety state.

### Workshop and community content

Space Engineers shows the value of shareable blueprints, scenarios, scripts, and worlds.

FormFactor community content should carry:

- author/origin;
- version;
- license;
- required catalogue versions;
- dependencies;
- trust state;
- validation state;
- whether scripts are present;
- whether physical assets are verified, partial, or visual-only.

### Engineering navigation markers

Space Engineers GPS is a useful interaction model. FormFactor should support saved **inspection markers** and **measurement bookmarks** for large projects:

- important net;
- test point;
- failed rule location;
- component to inspect;
- measurement target;
- repair location.

A bookmark should work across schematic, PCB, instrument, and 3D views when the object exists in those views.

## Extreme-scale world research

There is no single honest top-three ranking for the "largest game map" because games measure scale differently. A 1:1 galaxy, a procedurally generated universe, and a chunked terrain world are not directly comparable.

For FormFactor, the useful references are three different scale problems:

### Elite Dangerous - galactic indexing and navigation

Elite Dangerous models roughly 400 billion star systems in a 1:1 Milky Way-scale playspace. The lesson is not that FormFactor needs a giant world. The lesson is how to make a huge data space navigable.

FormFactor translation:

- hierarchical project indexing;
- strong search and filtering;
- fast jump-to-object navigation;
- region/subsystem grouping;
- bookmarks and recent locations;
- overview -> subsystem -> component zoom levels;
- avoid loading every detail at the same time.

Large future projects should feel searchable, not endless.

### No Man's Sky - procedural generation, streaming, LOD, and seamless transitions

No Man's Sky uses procedural generation at enormous scale and has repeatedly improved terrain generation, LOD, texture streaming, memory use, and seamless travel.

FormFactor translation:

- stream heavy visual assets only when needed;
- use multiple levels of visual detail for large assemblies;
- preserve project truth while presentation data loads/unloads;
- generate thumbnails/previews separately from authoritative data;
- keep transitions between 2D, PCB, instrument, and 3D views fast;
- use procedural presentation only where it does not create fake engineering facts.

### Minecraft - chunks and separate render/simulation distance

Minecraft divides worlds into chunks and separates what is rendered from what is actively simulated. That is one of the cleanest transferable ideas for very large FormFactor projects.

FormFactor translation:

- divide a huge design into logical **project regions**;
- render far-away regions at reduced detail;
- run expensive live visualization only where needed;
- allow explicit always-active analysis regions for monitored subsystems;
- keep global validation dependencies correct even when a region is not visually loaded;
- separate **presentation distance** from **analysis scope**.

Most importantly, FormFactor must never copy the common game shortcut of simply stopping important engineering truth outside the active visual area. A hidden subsystem may be unloaded visually, but its authoritative project state still exists.

## Combined new FormFactor requirements

This research adds the following future requirements:

1. **Design Scouting Report** showing known strengths, limits, missing evidence, and blocked requirements before final validation.
2. **Explicit analysis profiles** where supported, with fast modes clearly separated from final validated evidence.
3. **Project Evolution Timeline** for important reproducible engineering changes.
4. **Engineering Blueprints** with versioned identity, dependencies, provenance, and stale-result handling.
5. **Repair-state modeling** so construction, faults, diagnosis, repair, and re-validation are one continuous system.
6. **Inspection markers and measurement bookmarks** shared across linked views.
7. **Project-region streaming** for very large designs, separating presentation loading from authoritative state.
8. **Hierarchical navigation** from whole project to subsystem to component.
9. **LOD for presentation assets** without changing validation quality.
10. **Community engineering content metadata** covering license, dependencies, scripts, provenance, and trust.

## Anti-patterns to avoid

- Do not turn component selection into arbitrary RPG stats.
- Do not let progression points substitute for actual engineering capability.
- Do not use procedural generation to invent missing component facts.
- Do not make very large project spaces difficult to search.
- Do not simulate only what is visible if doing so changes required engineering results.
- Do not allow creator scripts to write authoritative pass/fail state.
- Do not reuse validation evidence after a blueprint is changed unless the dependency rules prove that evidence is still valid.
- Do not make constrained modes use easier physics than FreeLab.

## Future acceptance tests

- A Design Scouting Report distinguishes known, unknown, failed, and unsupported requirements.
- Changing a component invalidates scouting information that depended on that component.
- Preview analysis cannot be mistaken for final validation.
- A saved Engineering Blueprint preserves exact part and dependency identity.
- Inserting a blueprint into a new project marks project-specific old evidence stale when required.
- An injected repair fault is reproducible from the scenario record.
- A repaired project must pass the same normal validators as a never-damaged project.
- A saved inspection marker resolves to the same object across supported views.
- Unloading a 3D region does not delete or alter engineering state.
- Reduced LOD does not change pass/fail results.
- Search can locate a known component/net/subsystem without requiring the user to manually pan through the full project.
- Community content with missing origin/license/dependency metadata cannot enter a trusted catalogue state.

## Research starting points

- NBA 2K26 MyPLAYER Builder, 2K Newsroom: https://newsroom.2k.com/news/nbar-2k26-myplayer-builder-delivers-all-new-animation-glossary-scouting-reports-and-build-by-badges-for-increased-customization
- NBA 2K26 presentation systems, 2K Newsroom: https://newsroom.2k.com/news/latest-presentation-enhancements-bring-lifelike-atmosphere-and-authenticity-to-nba-2k26r
- NBA 2K26 MyNBA/MyGM simulation options, 2K Newsroom: https://newsroom.2k.com/news/endless-possibilities-await-as-mynba-levels-up-in-nbar-2k26
- Spore review and creator-system discussion, GameSpot.
- Space Engineers official feature list: https://www.spaceengineersgame.com/features/
- Elite Dangerous galaxy-scale documentation and official/community material describing its roughly 400-billion-system Milky Way.
- No Man's Sky official update notes covering procedural generation, LOD, streaming, memory, and seamless world presentation: https://www.nomanssky.com/
- Minecraft documentation describing chunks plus separate render and simulation distances.

These sources are references for general product design. Proprietary implementation is not copied into FormFactor.