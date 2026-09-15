# MLB The Show Design Research for FormFactor

Status: design reference only. FormFactor adopts general interaction, training, progression, feedback, and presentation ideas. It does not copy MLB The Show code, art, audio, branding, broadcast packages, player likenesses, commentary, menus, animations, or other protected content.

## What MLB The Show does especially well

Across many entries in the series, several design strengths appear repeatedly:

- **Practice is part of the game, not a separate manual.** Batting cages, pitching practice, fielding drills, and focused training let players improve one skill at a time.
- **Difficulty can scale without changing the sport.** Easier settings widen timing and input tolerance, while harder settings demand more precise execution. The underlying baseball rules remain the same.
- **Performance feedback is immediate and specific.** The game evaluates timing, pitch recognition, contact quality, fielding decisions, and other actions instead of only showing win/loss.
- **Progression can reward good process, not only perfect outcomes.** Some versions award development credit for strong at-bats, working counts, or other good decisions even when the final result is not ideal.
- **Focused training responds to weaknesses.** Older Road to the Show designs could react to recent struggles and offer targeted drills.
- **Presentation makes repeated actions feel important.** Broadcast-like camera work, sound, crowd reaction, overlays, and varied animations help routine actions feel meaningful.
- **Deep customization helps different skill levels.** Difficulty settings and sliders allow players to tune challenge without requiring one universal control scheme.
- **Career progression gives small actions long-term meaning.** Repeated performance, training, and decisions build toward a larger player-development arc.

## FormFactor translation

### 1. Engineering Practice Lab

Add focused practice modes that isolate one engineering skill at a time. Examples:

- identify resistor values;
- place a component correctly;
- route one clean trace;
- measure voltage with a virtual multimeter;
- find an open circuit;
- find a short circuit;
- identify a bad ground;
- diagnose a failed rail;
- read a waveform;
- identify a pinout mismatch;
- repair a deliberately faulted board.

Practice uses the same engineering core as FreeLab and Engineering Contracts. It never uses fake success rules.

### 2. Adaptive assistance, not adaptive engineering truth

FormFactor may change how much help the player receives, but it must never change whether a circuit is physically valid.

Assistance levels may change:

- hint frequency;
- highlighted candidate parts;
- pin labels;
- measurement suggestions;
- allowed tooltips;
- visible reference diagrams;
- time pressure;
- number of injected faults;
- how much of the expected workflow is shown.

Assistance levels must **not** change:

- electrical limits;
- ERC/DRC results;
- simulation results;
- safety gates;
- manufacturing rules;
- pass/fail engineering truth.

### 3. Action feedback card

After an important action, FormFactor should be able to show a small feedback card containing:

- what the player did;
- what changed;
- whether the action was valid, invalid, incomplete, or still unknown;
- the engineering evidence behind that state;
- one short next-step suggestion when appropriate.

Examples:

- `Probe placed correctly. Measured 3.29 V. Expected range: 3.20-3.40 V.`
- `Trace width fails the current rule. Required minimum: 0.40 mm. Actual: 0.25 mm.`
- `Good diagnostic step. The measurement ruled out the 5 V input rail.`

The feedback layer explains core results; it does not invent them.

### 4. Process-aware scoring

Engineering training should recognize good diagnostic process, not only final repair completion.

Possible evidence-backed metrics include:

- correct tool selection;
- correct measurement point;
- safe procedure order;
- number of unnecessary part swaps;
- number of unsupported guesses;
- fault isolation steps;
- correct interpretation of measured values;
- validation checks completed after repair;
- time to diagnosis only when timing is relevant to the exercise.

A good process score can coexist with a failed final design, but it can never convert that failed design into a passing engineering result.

### 5. Targeted drills from recent mistakes

When reproducible player history shows a repeated problem, FormFactor may recommend a matching practice drill.

Examples:

- repeated polarity mistakes -> polarity/orientation drill;
- repeated ground-reference errors -> grounding drill;
- repeated overcurrent trace failures -> trace/current-rule drill;
- repeated wrong meter range -> multimeter-range drill;
- repeated missed shorts -> continuity-testing drill.

Recommendations must be based on recorded player actions and validation evidence, not guessed skill weaknesses.

### 6. Difficulty profiles

Use assistance profiles rather than weakened physics.

Suggested profiles:

- **Learn**: strong hints, visible labels, guided measurements, no time pressure.
- **Standard**: normal hints, partial labels, normal workflow guidance.
- **Advanced**: fewer hints, fewer labels, more ambiguous fault symptoms.
- **Expert**: minimal guidance, realistic documentation, multiple plausible causes, strict workflow evidence.

Players should also be able to tune individual help features instead of being forced into one preset.

### 7. Career-style mastery path

Borrow the long-term satisfaction of Road to the Show without copying its structure or presentation.

A FormFactor mastery profile may track evidence-backed experience in areas such as:

- electrical fundamentals;
- component identification;
- schematic reading;
- soldering/repair concepts;
- multimeter use;
- oscilloscope use;
- PCB layout;
- power systems;
- digital logic;
- firmware/hardware debugging;
- manufacturing/DFM;
- semiconductor-device fundamentals.

Progress comes from validated completed work and practice evidence. It is not a cosmetic level that overrides engineering capability.

### 8. Broadcast-quality engineering presentation

MLB The Show demonstrates how strong presentation can make repeated technical actions feel important. FormFactor should use that idea in an original engineering style:

- smooth camera moves into a selected board area;
- clean close-up inspection views;
- clear measurement overlays;
- short replay of the action that caused a fault;
- before/after comparison after a repair;
- compact milestone cards for major project events;
- subtle audio/haptic confirmation for valid interactions;
- varied but restrained transition animation.

Presentation must stay secondary to readability and engineering evidence.

## Anti-patterns to avoid

Research into the series also highlights useful warnings:

- Do not hide important progression rules behind unexplained icons or perk systems.
- Do not make training so rare that players cannot practice a weakness when they need to.
- Do not over-reward weak outcomes simply because a partial action looked good.
- Do not force long loading or repeated setup around a short training exercise.
- Do not make one progression mode depend on unrelated systems when a player wants a focused learning path.

## Acceptance tests for future implementation

- A practice drill can launch directly from the skill it teaches.
- Practice and normal gameplay use the same engineering validators.
- Changing assistance level does not change engineering pass/fail results.
- Feedback names the exact evidence used for the result.
- A process score cannot override a failed validation gate.
- A repeated documented mistake can trigger a relevant drill suggestion.
- Players can disable targeted drill suggestions.
- Difficulty/help settings can be adjusted independently where practical.
- A completed repair requires final validation, not only a successful intermediate measurement.
- Presentation effects can be reduced without removing required engineering information.

## Research sources

- Official MLB The Show ecosystem and current game/update material.
- GameSpot reviews of MLB 09, MLB 10, MLB 11, MLB 13, MLB 14, MLB The Show 18, and MLB The Show 25.
- MLB The Show community feedback used only to identify recurring UX problems such as unclear progression and poorly explained systems.

These references are used for high-level product design only. No proprietary implementation is copied into FormFactor.
