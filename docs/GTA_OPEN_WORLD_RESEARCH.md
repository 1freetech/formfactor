# GTA and Open-World Design Research for FormFactor

Status: design research and implementation guidance. Grand Theft Auto titles and related open-source reimplementations are references for general game-design and software-architecture ideas only. FormFactor must not copy Rockstar or Take-Two assets, maps, missions, characters, dialogue, music, branding, distinctive UI, proprietary data, or reverse-engineered source code.

## Why this research matters

The Grand Theft Auto series is useful to study because it repeatedly combines two experiences in the same product: a structured mission path and a free sandbox. FormFactor needs the same basic balance in an engineering context. A player should be able to follow guided engineering contracts, but should also be able to enter FreeLab and experiment without being forced through a story or tutorial.

The most useful GTA lesson is not crime, vehicles, or story. It is the way a large set of systems can feel like one coherent playground while still giving the player clear goals, fast feedback, persistent progress, and room to make their own decisions.

## Released-series lessons and FormFactor translation

| GTA title / era | Useful design lesson | FormFactor translation |
| --- | --- | --- |
| **Grand Theft Auto (1997/1998)** | The original game made freedom itself the attraction: the player could ignore the next assignment and explore the city on their own. | Keep **FreeLab available from the start**. Guided training must never be the only way to use the simulator. |
| **GTA: London 1969 / London 1961** | A familiar system could feel different through a strong setting and scenario design. The harsh timers also show how punishment can become frustrating when retry cost is too high. | Reuse the same engineering core across themed labs and scenario packs. Use time pressure only as an optional challenge, and give fast checkpoints/retries. |
| **Grand Theft Auto 2** | The respect system made the world react to prior choices and exposed those relationships through visible meters. | Add **discipline mastery/proficiency** that reflects completed, validated work in areas such as power, digital, PCB layout, troubleshooting, and measurement. Never award mastery for cosmetic actions. |
| **Grand Theft Auto III** | GTA III proved that structured missions and a 3D sandbox could coexist without making either mode feel secondary. | Engineering Contracts and FreeLab share the same project format, controls, parts, instruments, and truth layer. The player can leave a guided contract and inspect or experiment without loading a different simulator. |
| **Vice City** | Strong art direction and coherent atmosphere made the world memorable even when the underlying mechanics were familiar. | Give FormFactor a consistent lab/workbench visual identity, readable lighting, strong materials, clear audio feedback, and distinct training environments without copying any GTA style, music, or imagery. |
| **San Andreas** | Variety, customization, skill systems, and a large number of activities made the sandbox feel broad and replayable. | Expand through **meaningful engineering activities**: schematic capture, PCB layout, instruments, firmware, repair, optimization, thermal/power work, and manufacturing checks. Avoid feature bloat: every activity must connect to the engineering truth layer. |
| **GTA Advance** | The portable version showed that shrinking a big game to weaker hardware can preserve the concept but lose usability if controls, camera, or presentation suffer. | `formfactor 2D Lite` must preserve the complete engineering loop and truth gates. Reduce rendering cost, not usability or correctness. |
| **Liberty City Stories** | A console-like experience on constrained hardware showed the value of scope control and platform-specific optimization. | Use lazy loading, compact assets, quality levels, and the same project files across Full and Lite profiles. |
| **Vice City Stories** | Reusing a known world while adding new progression systems showed that existing environments can support new goals without rebuilding everything. | Reuse the same workbench and component ecosystem for new training modules, troubleshooting scenarios, and project briefs. New content should mostly be data/contracts, not separate engines. |
| **Grand Theft Auto IV** | The game emphasized a more believable, cohesive world and consistent physical presentation rather than simply adding more activities. | Prefer **credible engineering behavior and tactile instrument feedback** over spectacle. Realistic-looking motion or graphics must never contradict solver or measurement results. |
| **The Lost and Damned / The Ballad of Gay Tony / Episodes from Liberty City** | The same city supported very different tones, activities, and perspectives while keeping the core world intact. | Let one FormFactor project be viewed through multiple engineering perspectives: schematic, board, 3D inspection, instrument, firmware, and validation views. All views share one authoritative project state. |
| **Chinatown Wars** | A smaller screen and top-down presentation still delivered depth by using clear interaction, short tactile tasks, and systems designed for the platform. | Make the 2D workbench a first-class experience. Direct actions such as place, wire, probe, measure, inspect, and repair should be quick and readable without requiring 3D. |
| **Grand Theft Auto V** | Multi-stage missions, preparation, role changes, optional approaches, random world events, and quick perspective switching made complex jobs easier to understand and more replayable. | Build **multi-stage Engineering Contracts**: plan -> choose parts -> build -> power -> test -> diagnose -> repair -> validate. Allow multiple valid engineering solutions. Switch views without losing selection, camera target, measurements, or current objective. Add optional fault/troubleshooting events later. |
| **GTA Online** | A persistent sandbox can support repeated activities, evolving content, and collaboration over a long period. | Long term, shared projects and cooperative lab roles may be useful, but only after deterministic save/load/share formats and security boundaries are stable. Do not copy GTA Online's economy or monetization model. |

## Systems to implement in FormFactor

### 1. FreeLab is always available

The player can build and test freely without completing lessons first. Guided content may recommend a path, but it cannot lock the simulator behind progression unless a specific feature genuinely depends on later engineering capability.

### 2. Engineering Contracts

A contract is a data-driven engineering challenge with:

- a clear brief;
- explicit constraints and available evidence;
- a small objective graph instead of one long scripted sequence;
- required and optional objectives;
- checkpoints at meaningful engineering milestones;
- fast retry from the failed stage;
- support for multiple valid solutions when the truth layer accepts them;
- a final result generated from implemented validators/solvers, never from a scripted success flag.

The default contract flow should mirror the minimum playable loop:

**Place -> Connect -> Power -> Test -> Diagnose -> Repair -> Validate**

### 3. Context-preserving view switching

Switching between schematic, PCB, 3D, instrument, firmware, and validation views should preserve:

- selected component/net;
- current objective;
- measurement target;
- validation state;
- undo/redo history where technically valid;
- relevant camera/zoom context.

This is FormFactor's useful equivalent of GTA V's perspective switching: different views of the same active situation, not different copies of the project.

### 4. Engineering mastery instead of arbitrary XP

Borrow the useful part of GTA2's visible reputation idea without copying the fiction. Track mastery only from objective evidence such as completed validated contracts, correct measurements, successful diagnoses, safe repair procedures, or demonstrated design constraints.

Possible mastery domains:

- electrical fundamentals;
- power;
- digital logic;
- PCB layout;
- instruments and measurement;
- troubleshooting/repair;
- firmware;
- manufacturing/DFM;
- advanced signal/power/thermal work when those solvers exist.

Mastery is educational feedback, not engineering truth. It must never cause an invalid design to pass.

### 5. Fast checkpoints and retry

A failed contract should normally restart from the nearest valid engineering checkpoint rather than forcing the entire exercise to be repeated. Save the exact project state and the validation/solver provenance needed for reproducible replay.

### 6. Optional dynamic fault events

Later training modules can inject explicit, reproducible faults such as:

- wrong value/component variant;
- open connection;
- shorted net;
- reversed polarity;
- missing power rail;
- firmware configuration error;
- out-of-range measurement;
- thermal or current-limit problem when the required model exists.

Every injected fault must be declared in the scenario record. The game cannot secretly alter engineering state without a reproducible record.

### 7. On-demand world/workbench loading

Borrow the general open-world performance principle, not GTA code. Heavy 3D assets, thumbnails, large waveforms, optional environments, and inspection detail should load when needed and unload when safely inactive. The project/truth state stays resident independently of presentation assets.

### 8. Full/Lite parity

The Full and `2D Lite` profiles must open the same project and use the same truth/validation rules. Lite may omit expensive presentation features but may not weaken ERC, DRC, solver requirements, provenance, or export gates.

## Open-source GTA-related projects: what is safe to learn from

### OpenRW

OpenRW is an open-source GTA III executable/engine reimplementation. Its README states that it is cross-platform, requires a legitimate copy of the original game data, and is licensed under **GPLv3 or later**. Its stated goals include modern gamepad compatibility, save-game compatibility, and compatibility with data-only mods.

Useful architecture lessons for FormFactor:

- keep platform/input concerns separate from game state;
- keep script/objective logic separate from rendering;
- treat save compatibility as a first-class contract;
- make controller support part of the engine plan, not a late patch;
- build cross-platform behavior around portable core state.

**FormFactor rule:** study OpenRW's public architecture and behavior, but do not copy GPL code into FormFactor under the current licensing plan unless the project deliberately accepts the resulting GPL obligations.

### re3 / reVC

The re3/reVC projects reconstructed GTA III and Vice City source behavior through reverse engineering. Their commonly mirrored README says the reversed code does **not** have a normal reusable license and should be treated as educational/documentation/modding material. The projects also require original game assets.

Useful lessons:

- renderer replacement can be separated from game logic;
- old software can be made cross-platform by isolating platform-specific layers;
- configuration, input, audio, world state, camera, and scripting benefit from clear subsystem boundaries;
- preserving behavior while replacing presentation technology is possible when the state contracts are understood.

**FormFactor rule:** do not copy, vendor, port, or derive code from re3/reVC. Use it only as a high-level architecture/history reference.

### librw

`librw` is a separate MIT-licensed reimplementation of much of the RenderWare graphics engine. It demonstrates a clean idea that is relevant to FormFactor: keep rendering/data-format responsibilities behind a library boundary and support more than one graphics backend.

FormFactor already plans to use **bgfx** for the production renderer, so `librw` is not proposed as a dependency. Its architecture is useful as validation that a renderer abstraction can remain separate from simulation/game state.

### Forks with unusual or custom licenses

Some current reVC/re3 forks advertise custom licensing terms such as "MIT with Proof-of-Usage" while still describing themselves as derivative/reverse-engineered works. Do not treat those forks as automatically safe dependencies. Any future third-party code import requires a separate license and provenance review.

## What FormFactor must not copy

Do not copy or imitate protected GTA expression, including:

- city/map layouts;
- mission scripts or story beats;
- characters or dialogue;
- logos, names, typography, loading-screen compositions, HUD layouts, or branded visual identity;
- radio stations, licensed music, voice lines, sound effects, or cutscenes;
- vehicles or other proprietary art assets;
- GTA game data or RenderWare assets;
- reverse-engineered re3/reVC source code.

General ideas such as sandbox freedom, checkpoints, mission graphs, persistent state, view switching, lazy loading, controller support, and data-driven scenarios are reusable design patterns and should be implemented originally for FormFactor.

## Priority in the existing FormFactor roadmap

This research does **not** replace the current top three priorities:

1. bulletproof the build/test pipeline;
2. lock the minimum playable engineering loop;
3. formalize the engineering-truth/component-data model.

The GTA-derived work fits underneath those priorities in this order:

1. define the Engineering Contract state model around the minimum playable loop;
2. make FreeLab and guided contracts use the same project/truth state;
3. add checkpoint/retry records after save-state versioning exists;
4. add context-preserving schematic/PCB/instrument view switching;
5. add `2D Lite` parity and lazy presentation loading;
6. add reproducible dynamic fault scenarios;
7. consider collaboration/shared projects only after deterministic save/share and security are mature.

## Research sources

- GameSpot, **Every Grand Theft Auto Game, Reviewed: GTA 5, San Andreas, And More**: https://www.gamespot.com/gallery/every-grand-theft-auto-game-reviewed-gta-5-san-andreas-and-more/2900-2186/
- GameSpot, **Grand Theft Auto V Review**: https://www.gamespot.com/reviews/grand-theft-auto-v-review/1900-6414475/
- Metacritic series pages and critic aggregates for GTA III, Vice City, San Andreas, Liberty City Stories, Vice City Stories, GTA IV, Chinatown Wars, Episodes from Liberty City, and GTA V.
- OpenRW: https://github.com/rwengine/openrw
- librw: https://github.com/aap/librw
- re3/reVC mirrors were reviewed only as architecture/history references; their code is not approved for reuse in FormFactor.
