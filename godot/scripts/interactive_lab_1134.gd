extends "res://scripts/interactive_lab_1132.gd"

# FormFactor 1.134 visual-realism layer.
# Geometry is presentation-only: it may represent sourced/package-standard dimensions,
# but it never creates electrical, thermal, timing, manufacturing, package, or simulation truth.
# Authoritative geometry source unit is millimeters. 1 Godot world unit = 10 mm.
const MM_TO_WORLD := 0.1
const BOARD_WIDTH_MM := 80.0
const BOARD_DEPTH_MM := 50.0
const BOARD_THICKNESS_MM := 1.6
const BOARD_TOP_Y := 0.24

func mm(value: float) -> float:
    return value * MM_TO_WORLD

func mm3(x: float, y: float, z: float) -> Vector3:
    return Vector3(mm(x), mm(y), mm(z))

func _ready() -> void:
    super._ready()
    if status_label != null:
        status_label.text = "3D PCB // dimensioned in mm // schematic is context only"

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var version_label := get_node_or_null("WorkbenchVersion3D") as Label3D
    if version_label != null:
        version_label.text = "1.134 // DIMENSIONED 3D PCB"

func _build_board() -> void:
    # 80 x 50 x 1.6 mm teaching board; thickness is the common nominal FR-4 board value.
    var center_y: float = BOARD_TOP_Y - mm(BOARD_THICKNESS_MM) * 0.5
    var fr4 := _material(Color(0.018, 0.19, 0.075, 1.0), 0.02, 0.52)
    _box("PCB_Substrate", mm3(BOARD_WIDTH_MM, BOARD_THICKNESS_MM, BOARD_DEPTH_MM), Vector3(0, center_y, 0), fr4)

    # Exposed board edge reads as FR-4 rather than a thick toy slab.
    var edge := _material(Color(0.31, 0.29, 0.18, 1.0), 0.0, 0.72)
    _box("PCB_EdgeNorth", mm3(BOARD_WIDTH_MM, BOARD_THICKNESS_MM, 0.18), Vector3(0, center_y, -mm(BOARD_DEPTH_MM) * 0.5), edge)
    _box("PCB_EdgeSouth", mm3(BOARD_WIDTH_MM, BOARD_THICKNESS_MM, 0.18), Vector3(0, center_y, mm(BOARD_DEPTH_MM) * 0.5), edge)

    # Four M3 mounting holes: 3.2 mm finished opening with 6.0 mm copper annulus.
    var copper := _material(Color(0.78, 0.42, 0.13, 1.0), 0.82, 0.24)
    var void_mat := _material(Color(0.006, 0.008, 0.009, 1.0), 0.0, 1.0)
    for p in [Vector2(-35, -20), Vector2(35, -20), Vector2(-35, 20), Vector2(35, 20)]:
        _cylinder("MountCopper", mm(3.0), mm(0.035), Vector3(mm(p.x), BOARD_TOP_Y + mm(0.018), mm(p.y)), copper)
        _cylinder("MountHole", mm(1.6), mm(BOARD_THICKNESS_MM + 0.08), Vector3(mm(p.x), center_y, mm(p.y)), void_mat)

func _build_resistor(body: Node3D, body_material: Material, metal: Material) -> void:
    # Generic DIN-style 0.25 W axial teaching package: 6.3 x 2.3 mm body,
    # 0.6 mm leads, 10.16 mm nominal mounting pitch. Not a manufacturer claim.
    var y: float = mm(2.0)
    var core := _cylinder_child(body, "AxialResistorBody3D", mm(1.15), mm(6.3), Vector3(0, y, 0), body_material)
    core.rotation_degrees.z = 90.0
    for x in [-mm(4.1), mm(4.1)]:
        var lead := _cylinder_child(body, "ResistorLead3D", mm(0.30), mm(1.9), Vector3(x, mm(0.9), 0), metal)
        lead.rotation_degrees.z = 90.0
    for x in [-mm(5.08), mm(5.08)]:
        _cylinder_child(body, "ResistorPin3D", mm(0.30), mm(1.8), Vector3(x, mm(0.9), 0), metal)

func _build_dip(body: Node3D, pins_per_side: int, black: Material, metal: Material) -> void:
    # Generic through-hole DIP: 2.54 mm pin pitch, 7.62 mm row spacing.
    # Body length follows pin count; package is explicitly generic, not manufacturer-exact.
    # Keep the historical node identity so cross-version visual regression checks remain valid.
    var body_length_mm: float = maxf(9.8, float(pins_per_side - 1) * 2.54 + 2.2)
    _box_child(body, "DIPPackage3D", mm3(body_length_mm, 3.3, 6.35), Vector3(0, mm(2.05), 0), black)
    for side in [-1.0, 1.0]:
        for i in range(pins_per_side):
            var x_mm: float = (float(i) - float(pins_per_side - 1) * 0.5) * 2.54
            var z_mm: float = side * 3.81
            _box_child(body, "DIPPin3D", mm3(0.50, 1.8, 0.25), Vector3(mm(x_mm), mm(0.9), mm(z_mm)), metal)

func _build_to92(body: Node3D, black: Material, metal: Material) -> void:
    # Generic TO-92 teaching envelope: 4.8 x 4.0 x 5.0 mm, 1.27 mm lead pitch.
    _box_child(body, "TO92Body3D", mm3(4.8, 5.0, 4.0), Vector3(0, mm(3.1), 0), black)
    for i in range(3):
        var x_mm: float = (float(i) - 1.0) * 1.27
        _cylinder_child(body, "TO92Lead3D", mm(0.22), mm(2.2), Vector3(mm(x_mm), mm(1.1), 0), metal)

func _build_led_component(body: Node3D, metal: Material) -> void:
    # Generic T-1 3 mm LED envelope with 2.54 mm lead spacing.
    # Keep the historical LedLens3D identity so schematic/package regression mapping stays stable.
    var glass := _material(Color(0.75, 0.08, 0.04, 0.42), 0.05, 0.16)
    glass.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
    _cylinder_child(body, "LedLens3D", mm(1.5), mm(4.5), Vector3(0, mm(3.0), 0), glass)
    for x_mm in [-1.27, 1.27]:
        _cylinder_child(body, "LEDLead3D", mm(0.25), mm(2.0), Vector3(mm(x_mm), mm(1.0), 0), metal)
