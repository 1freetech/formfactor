# FormFactor 1.127 — Open-Source Integration

This update turns the four reviewed repositories into real FormFactor work.
It does not paste whole projects into the game. Each idea is rebuilt around
FormFactor's rule: **the renderer can show a result, but the C++ engineering
core decides engineering truth.**

## The ten implemented focus points

1. **Pin-to-pin wiring:** every current component family now has named pins.
   New wires remember both component names and exact pin IDs, such as
   `PWR1.POS -> R1.1` and `D1.K -> PWR1.NEG`.

2. **Graph-backed nets:** the Godot editor now builds a deterministic
   undirected pin graph and converts connected pin groups into named nets.
   This is a gameplay/editor topology layer, not an electrical solver.

3. **Real editor tool states:** FormFactor now exposes SELECT, PLACE, WIRE,
   MOVE, ROTATE, INSPECT, and TEST modes in one compact toolbar. The existing
   drag/build controls stay available.

4. **Digital truth tables:** the C++ digital simulator now supports NOT, AND,
   OR, NAND, NOR, XOR, and XNOR. It can generate complete Low/High truth
   tables. The Godot truth-table panel is clearly marked as a preview while
   the C++ simulator remains authoritative.

5. **Reusable Engineering Blueprints:** the current board can be saved and
   restored from a versioned JSON blueprint. Exact pin endpoints survive the
   round trip.

6. **KiCad import groundwork:** a new C++ legacy Eeschema importer preserves
   explicit reference designator, symbol library ID, value, and footprint
   library ID. Regression data uses MIT-licensed PCB-Design examples.

7. **Symbol -> footprint -> package links:** placed parts can carry explicit
   symbol, footprint, package-style, and verification-status metadata.
   Reference-only examples stay labeled reference-only instead of being
   treated as verified physical truth.

8. **Strict object checks:** the 1.127 pin catalogue is validated before use.
   Empty pin lists, duplicate/unsafe pin IDs, and unsupported pin types fail
   validation rather than being silently accepted.

9. **Simulation/presentation boundary:** the game now writes a deterministic
   `formfactor-editor-snapshot-v1` file containing parts, exact pin wires,
   nets, and metadata. The file explicitly states
   `engineering_truth = cpp_core_required`.

10. **Licensing and attribution:** FormFactor now has a top-level MIT license
    and a Third-Party Notices file. GPLv3 Logic-Circuit-Simulator code and the
    unlicensed GodotAUVSim source were not copied.

## Pin-level LED precheck

The old visual test could find a path between whole component objects. 1.127
adds a stricter topology precheck. A simple LED loop must have a route from the
power positive pin to the LED anode and a second route from the LED cathode
back to the power negative pin. Resistors, inductors, and fuses have explicit
pass-through topology for this preview.

Passing this check does **not** prove voltage, current, power rating,
temperature, polarity safety, SPICE behavior, or manufacturability. Those
claims still belong to implemented C++ validation and solver stages.

## Why this matters

The board the player sees and the circuit the engineering core understands are
now much closer to the same object. That makes later multimeter tools, shorts,
polarity errors, SPICE runs, PCB routing, fault diagnosis, and real electrical
feedback easier to add without inventing hidden connections.
