extends Control

# FormFactor 1.125 procedural schematic mirror.
# Symbols are drawn as vector primitives so the game does not depend on
# raster screenshots. Shapes follow common IEEE/IEC/KiCad conventions.
# This layer is visual only; engineering truth remains in the C++ core.

const FG := Color(0.82, 0.90, 0.92, 1.0)
const MUTED := Color(0.46, 0.58, 0.60, 1.0)
const NEON := Color(0.22, 1.0, 0.08, 1.0)
const BG := Color(0.007, 0.014, 0.018, 0.98)

var component_rows: Array = []
var wire_count := 0

func set_circuit(rows: Array, wires: int) -> void:
    component_rows = rows.duplicate(true)
    wire_count = wires
    queue_redraw()

func _draw() -> void:
    draw_rect(Rect2(Vector2.ZERO, size), BG)
    var font := ThemeDB.fallback_font
    if component_rows.is_empty():
        draw_string(font, Vector2(12, 24), "SCHEMATIC", HORIZONTAL_ALIGNMENT_LEFT, -1, 12, NEON)
        draw_string(font, Vector2(12, 50), "Place a part to see its standard symbol.", HORIZONTAL_ALIGNMENT_LEFT, -1, 11, MUTED)
        draw_string(font, Vector2(12, size.y - 12), "WIRES  0", HORIZONTAL_ALIGNMENT_LEFT, -1, 10, MUTED)
        return

    var visible_count := mini(component_rows.size(), 7)
    for i in range(visible_count):
        var row: Dictionary = component_rows[i]
        var y := 27.0 + float(i) * 45.0
        _draw_symbol(str(row.get("id", "")), Vector2(18.0, y))
        draw_string(font, Vector2(126, y + 4), str(row.get("refdes", "?")), HORIZONTAL_ALIGNMENT_LEFT, 48, 11, NEON)
        draw_string(font, Vector2(176, y + 4), str(row.get("name", "")), HORIZONTAL_ALIGNMENT_LEFT, 158, 10, FG)

    if component_rows.size() > visible_count:
        draw_string(font, Vector2(12, size.y - 27), "+ %d more parts" % (component_rows.size() - visible_count), HORIZONTAL_ALIGNMENT_LEFT, -1, 10, MUTED)
    draw_string(font, Vector2(250, size.y - 11), "WIRES  %d" % wire_count, HORIZONTAL_ALIGNMENT_LEFT, -1, 10, MUTED)

func _draw_symbol(component_id: String, origin: Vector2) -> void:
    match component_id:
        "resistor":
            _resistor(origin)
        "potentiometer":
            _potentiometer(origin)
        "ceramic_cap":
            _capacitor(origin, false)
        "electrolytic_cap":
            _capacitor(origin, true)
        "inductor":
            _inductor(origin)
        "diode":
            _diode(origin, false, false)
        "zener":
            _diode(origin, true, false)
        "led":
            _diode(origin, false, true)
        "npn":
            _bjt(origin, true)
        "pnp":
            _bjt(origin, false)
        "nmos":
            _mosfet(origin, true)
        "pmos":
            _mosfet(origin, false)
        "logic":
            _logic_inverter(origin)
        "opamp":
            _opamp(origin, false)
        "comparator":
            _opamp(origin, true)
        "regulator":
            _regulator(origin)
        "fuse":
            _fuse(origin)
        "switch":
            _switch(origin)
        "relay":
            _relay(origin)
        "connector":
            _connector(origin)
        "test_point":
            _test_point(origin)
        "sensor":
            _sensor(origin)
        "buzzer":
            _buzzer(origin)
        "power":
            _power(origin)
        "ground":
            _ground(origin)
        _:
            _generic_block(origin)

func _wire(a: Vector2, b: Vector2, width := 2.0) -> void:
    draw_line(a, b, FG, width, true)

func _arrow(a: Vector2, b: Vector2) -> void:
    _wire(a, b, 1.7)
    var direction := (b - a).normalized()
    var side := Vector2(-direction.y, direction.x)
    _wire(b, b - direction * 7.0 + side * 3.5, 1.7)
    _wire(b, b - direction * 7.0 - side * 3.5, 1.7)

func _resistor(o: Vector2) -> void:
    _wire(o + Vector2(0, 0), o + Vector2(20, 0))
    draw_rect(Rect2(o + Vector2(20, -8), Vector2(50, 16)), FG, false, 2.0)
    _wire(o + Vector2(70, 0), o + Vector2(94, 0))

func _potentiometer(o: Vector2) -> void:
    _resistor(o)
    _arrow(o + Vector2(66, -19), o + Vector2(48, -3))
    _wire(o + Vector2(66, -19), o + Vector2(66, -26))

func _capacitor(o: Vector2, polarized: bool) -> void:
    _wire(o, o + Vector2(38, 0))
    _wire(o + Vector2(44, -12), o + Vector2(44, 12), 2.2)
    _wire(o + Vector2(56, -12), o + Vector2(56, 12), 2.2)
    _wire(o + Vector2(56, 0), o + Vector2(94, 0))
    if polarized:
        var font := ThemeDB.fallback_font
        draw_string(font, o + Vector2(32, -11), "+", HORIZONTAL_ALIGNMENT_LEFT, -1, 11, FG)

func _inductor(o: Vector2) -> void:
    _wire(o, o + Vector2(20, 0))
    for i in range(4):
        draw_arc(o + Vector2(28 + i * 12, 0), 7.0, PI, TAU, 16, FG, 2.0, true)
    _wire(o + Vector2(70, 0), o + Vector2(94, 0))

func _diode(o: Vector2, zener: bool, led: bool) -> void:
    _wire(o, o + Vector2(28, 0))
    var triangle := PackedVector2Array([
        o + Vector2(28, -12),
        o + Vector2(58, 0),
        o + Vector2(28, 12),
        o + Vector2(28, -12)
    ])
    draw_polyline(triangle, FG, 2.0, true)
    if zener:
        _wire(o + Vector2(60, -12), o + Vector2(60, 12), 2.2)
        _wire(o + Vector2(60, -12), o + Vector2(66, -16), 2.2)
        _wire(o + Vector2(60, 12), o + Vector2(54, 16), 2.2)
    else:
        _wire(o + Vector2(60, -12), o + Vector2(60, 12), 2.2)
    _wire(o + Vector2(60, 0), o + Vector2(94, 0))
    if led:
        _arrow(o + Vector2(54, -18), o + Vector2(70, -30))
        _arrow(o + Vector2(64, -12), o + Vector2(80, -24))

func _bjt(o: Vector2, npn: bool) -> void:
    var c := o + Vector2(50, 0)
    draw_arc(c, 22, 0, TAU, 40, MUTED, 1.2, true)
    _wire(o + Vector2(6, 0), o + Vector2(38, 0))
    _wire(o + Vector2(38, -15), o + Vector2(38, 15), 2.2)
    _wire(o + Vector2(38, -8), o + Vector2(67, -22))
    _wire(o + Vector2(38, 8), o + Vector2(67, 22))
    if npn:
        _arrow(o + Vector2(51, 14), o + Vector2(65, 21))
    else:
        _arrow(o + Vector2(65, 21), o + Vector2(51, 14))

func _mosfet(o: Vector2, n_channel: bool) -> void:
    _wire(o + Vector2(8, 0), o + Vector2(32, 0))
    _wire(o + Vector2(32, -16), o + Vector2(32, 16), 2.2)
    _wire(o + Vector2(42, -15), o + Vector2(42, 15), 2.2)
    _wire(o + Vector2(42, -11), o + Vector2(70, -11))
    _wire(o + Vector2(42, 11), o + Vector2(70, 11))
    _wire(o + Vector2(70, -11), o + Vector2(70, -25))
    _wire(o + Vector2(70, 11), o + Vector2(70, 25))
    if n_channel:
        _arrow(o + Vector2(58, 0), o + Vector2(45, 0))
    else:
        _arrow(o + Vector2(45, 0), o + Vector2(58, 0))

func _logic_inverter(o: Vector2) -> void:
    _wire(o, o + Vector2(22, 0))
    var tri := PackedVector2Array([
        o + Vector2(22, -18),
        o + Vector2(22, 18),
        o + Vector2(66, 0),
        o + Vector2(22, -18)
    ])
    draw_polyline(tri, FG, 2.0, true)
    draw_circle(o + Vector2(72, 0), 5.0, BG)
    draw_arc(o + Vector2(72, 0), 5.0, 0, TAU, 20, FG, 2.0, true)
    _wire(o + Vector2(77, 0), o + Vector2(94, 0))

func _opamp(o: Vector2, comparator: bool) -> void:
    var tri := PackedVector2Array([
        o + Vector2(20, -22),
        o + Vector2(20, 22),
        o + Vector2(70, 0),
        o + Vector2(20, -22)
    ])
    draw_polyline(tri, FG, 2.0, true)
    _wire(o, o + Vector2(20, -10))
    _wire(o, o + Vector2(20, 10))
    _wire(o + Vector2(70, 0), o + Vector2(94, 0))
    var font := ThemeDB.fallback_font
    draw_string(font, o + Vector2(25, -6), "+", HORIZONTAL_ALIGNMENT_LEFT, -1, 10, FG)
    draw_string(font, o + Vector2(25, 15), "-", HORIZONTAL_ALIGNMENT_LEFT, -1, 10, FG)
    if comparator:
        draw_string(font, o + Vector2(44, 4), ">", HORIZONTAL_ALIGNMENT_LEFT, -1, 9, MUTED)

func _regulator(o: Vector2) -> void:
    _wire(o, o + Vector2(18, 0))
    draw_rect(Rect2(o + Vector2(18, -16), Vector2(56, 32)), FG, false, 2.0)
    _wire(o + Vector2(74, 0), o + Vector2(94, 0))
    var font := ThemeDB.fallback_font
    draw_string(font, o + Vector2(35, 4), "REG", HORIZONTAL_ALIGNMENT_LEFT, -1, 9, FG)

func _fuse(o: Vector2) -> void:
    _wire(o, o + Vector2(24, 0))
    draw_rect(Rect2(o + Vector2(24, -8), Vector2(46, 16)), FG, false, 2.0)
    _wire(o + Vector2(28, 0), o + Vector2(66, 0), 1.4)
    _wire(o + Vector2(70, 0), o + Vector2(94, 0))

func _switch(o: Vector2) -> void:
    _wire(o, o + Vector2(30, 0))
    _wire(o + Vector2(64, 0), o + Vector2(94, 0))
    draw_circle(o + Vector2(30, 0), 3.5, FG)
    draw_circle(o + Vector2(64, 0), 3.5, FG)
    _wire(o + Vector2(31, -2), o + Vector2(58, -18), 2.0)

func _relay(o: Vector2) -> void:
    draw_rect(Rect2(o + Vector2(4, -11), Vector2(30, 22)), FG, false, 2.0)
    draw_arc(o + Vector2(19, 0), 7, 0, TAU, 20, MUTED, 1.5, true)
    draw_circle(o + Vector2(57, 9), 3.0, FG)
    draw_circle(o + Vector2(84, 9), 3.0, FG)
    _wire(o + Vector2(58, 7), o + Vector2(78, -8), 1.8)

func _connector(o: Vector2) -> void:
    _wire(o, o + Vector2(18, 0))
    for i in range(4):
        draw_circle(o + Vector2(28 + i * 16, 0), 4.0, BG)
        draw_arc(o + Vector2(28 + i * 16, 0), 4.0, 0, TAU, 18, FG, 1.7, true)
    _wire(o + Vector2(80, 0), o + Vector2(94, 0))

func _test_point(o: Vector2) -> void:
    _wire(o, o + Vector2(48, 0))
    draw_circle(o + Vector2(52, 0), 6.0, BG)
    draw_arc(o + Vector2(52, 0), 6.0, 0, TAU, 24, FG, 2.0, true)
    _wire(o + Vector2(52, 6), o + Vector2(52, 20), 1.6)

func _sensor(o: Vector2) -> void:
    _wire(o, o + Vector2(24, 0))
    draw_circle(o + Vector2(50, 0), 22.0, BG)
    draw_arc(o + Vector2(50, 0), 22.0, 0, TAU, 36, FG, 2.0, true)
    _wire(o + Vector2(72, 0), o + Vector2(94, 0))
    var font := ThemeDB.fallback_font
    draw_string(font, o + Vector2(46, 4), "S", HORIZONTAL_ALIGNMENT_LEFT, -1, 10, FG)

func _buzzer(o: Vector2) -> void:
    _wire(o, o + Vector2(26, 0))
    draw_circle(o + Vector2(48, 0), 20.0, BG)
    draw_arc(o + Vector2(48, 0), 20.0, 0, TAU, 36, FG, 2.0, true)
    _wire(o + Vector2(68, 0), o + Vector2(78, 0))
    draw_arc(o + Vector2(76, 0), 9.0, -0.8, 0.8, 14, FG, 1.5, true)
    draw_arc(o + Vector2(78, 0), 16.0, -0.7, 0.7, 16, MUTED, 1.5, true)

func _power(o: Vector2) -> void:
    _wire(o, o + Vector2(33, 0))
    _wire(o + Vector2(40, -15), o + Vector2(40, 15), 2.5)
    _wire(o + Vector2(55, -9), o + Vector2(55, 9), 2.5)
    _wire(o + Vector2(55, 0), o + Vector2(94, 0))
    var font := ThemeDB.fallback_font
    draw_string(font, o + Vector2(34, -17), "+", HORIZONTAL_ALIGNMENT_LEFT, -1, 9, FG)
    draw_string(font, o + Vector2(52, -13), "-", HORIZONTAL_ALIGNMENT_LEFT, -1, 9, FG)

func _ground(o: Vector2) -> void:
    _wire(o + Vector2(48, -18), o + Vector2(48, 0))
    _wire(o + Vector2(30, 0), o + Vector2(66, 0), 2.2)
    _wire(o + Vector2(35, 7), o + Vector2(61, 7), 2.0)
    _wire(o + Vector2(41, 14), o + Vector2(55, 14), 1.8)

func _generic_block(o: Vector2) -> void:
    _wire(o, o + Vector2(20, 0))
    draw_rect(Rect2(o + Vector2(20, -14), Vector2(54, 28)), FG, false, 2.0)
    _wire(o + Vector2(74, 0), o + Vector2(94, 0))
