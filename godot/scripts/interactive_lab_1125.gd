extends "res://scripts/interactive_lab_1124.gd"

# FormFactor 1.125
# Electronics-specific 3D geometry + vector schematic symbols + a cleaner board.
# Rendering stays advisory/presentation-only. The validated C++ engineering core
# remains authoritative for electrical truth.

const SCHEMATIC_CANVAS = preload("res://scripts/schematic_symbol_canvas_1125.gd")
const METAL_COLOR := Color(0.72, 0.75, 0.78, 1.0)
const COPPER_COLOR := Color(0.72, 0.34, 0.10, 1.0)
const DARK_PACKAGE := Color(0.045, 0.052, 0.060, 1.0)
const GLASS_COLOR := Color(0.72, 0.88, 0.92, 0.34)

var schematic_canvas: Control

func _ready() -> void:
    super._ready()
    _strip_board_silkscreen()
    _install_vector_schematic()
    _simplify_static_hud_text()
    _refresh_schematic()
    if status_label != null:
        status_label.text = "Blank PCB ready. Pick or drag a component onto the board."
    if build_status != null:
        build_status.text = "BUILD / BUY"

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.124"):
                label.text = "FORMFACTOR 1.125"

func _strip_board_silkscreen() -> void:
    # The PCB is a blank construction canvas. Remove all inherited 3D words,
    # reference text, version stamps, and pre-drawn generic solder pads.
    for child in find_children("*", "Label3D", true, false):
        child.queue_free()
    for pad in find_children("SolderPad*", "MeshInstance3D", true, false):
        pad.queue_free()

func _simplify_static_hud_text() -> void:
    var hud := get_node_or_null("HUD") as CanvasLayer
    if hud == null:
        return

    var instruction_panel := hud.get_node_or_null("InstructionPanel") as ColorRect
    if instruction_panel != null:
        instruction_panel.size = Vector2(430, 74)
        for child in instruction_panel.get_children():
            if child is Label and child != status_label:
                child.visible = false
        if status_label != null:
            status_label.position = Vector2(14, 8)
            status_label.size = Vector2(402, 54)
            status_label.add_theme_font_size_override("font_size", 12)

    for child in hud.find_children("*", "Label", true, false):
        var label := child as Label
        if label == null:
            continue
        if label.text.begins_with("3D DISPLAY LAYER"):
            label.visible = false
        elif label.text.begins_with("LEFT CLICK PARTS"):
            label.visible = false
        elif label.text.begins_with("BOARD HISTORY"):
            label.visible = false

    var smart_panel := hud.get_node_or_null("SmartTargetPanel") as ColorRect
    if smart_panel != null:
        smart_panel.visible = false

func _install_vector_schematic() -> void:
    var hud := get_node_or_null("HUD")
    if hud == null:
        return
    var panel := hud.get_node_or_null("SchematicMirrorPanel") as ColorRect
    if panel == null:
        return
    if schematic_label != null:
        schematic_label.visible = false
    schematic_canvas = SCHEMATIC_CANVAS.new()
    schematic_canvas.name = "VectorSchematicCanvas1125"
    schematic_canvas.position = Vector2(8, 38)
    schematic_canvas.size = Vector2(352, 364)
    schematic_canvas.mouse_filter = Control.MOUSE_FILTER_IGNORE
    panel.add_child(schematic_canvas)

func _refresh_inventory() -> void:
    if inventory_list == null:
        return
    for child in inventory_list.get_children():
        child.queue_free()

    var query := ""
    if search_box != null:
        query = search_box.text.strip_edges().to_lower()
    var category := "All"
    if category_filter != null and category_filter.selected >= 0:
        category = category_filter.get_item_text(category_filter.selected)

    for component in COMPONENTS:
        if category != "All" and str(component.category) != category:
            continue
        var haystack := (str(component.name) + " " + str(component.prefix) + " " + str(component.category)).to_lower()
        if not query.is_empty() and haystack.find(query) == -1:
            continue

        var component_id := str(component.id)
        var count := int(inventory_stock.get(component_id, 0))
        var button := Button.new()
        button.name = "Inventory_%s" % component_id
        button.text = "%-3s  %-25s x%-2d" % [component.prefix, component.name, count]
        button.alignment = HORIZONTAL_ALIGNMENT_LEFT
        button.custom_minimum_size = Vector2(322, 36)
        button.disabled = count <= 0
        button.tooltip_text = "%s // %s\nClick to select. Hold and drag onto the PCB to place." % [component.name, component.category]
        button.add_theme_color_override("font_hover_color", NEON)
        if selected_component_id == component_id:
            button.add_theme_color_override("font_color", NEON)
        button.pressed.connect(select_component.bind(component_id))
        button.gui_input.connect(_catalog_tile_gui_input.bind(component_id))
        button.mouse_entered.connect(_catalog_tile_hover.bind(component_id))
        button.mouse_exited.connect(_catalog_tile_exit.bind(component_id))
        inventory_list.add_child(button)

func _update_selected_label() -> void:
    if selected_label == null:
        return
    if wire_mode:
        selected_label.text = "WIRE // choose two placed parts"
    elif selected_component_id.is_empty():
        selected_label.text = "Choose a component"
    else:
        var component := _component_by_id(selected_component_id)
        if component.is_empty():
            selected_label.text = "Choose a component"
        else:
            selected_label.text = "%s  %s" % [component.prefix, component.name]

func _refresh_schematic() -> void:
    if schematic_label != null:
        schematic_label.text = ""
    if schematic_canvas == null:
        return
    var rows: Array = []
    for body in placed_components:
        if body == null or not is_instance_valid(body):
            continue
        var component_id := str(body.get_meta("component_id", ""))
        var component := _component_by_id(component_id)
        rows.append({
            "id": component_id,
            "refdes": str(body.get_meta("refdes", "?")),
            "name": str(component.get("name", component_id))
        })
    schematic_canvas.set_circuit(rows, user_wires.size())

func _build_component_visual(body: StaticBody3D, component: Dictionary, _refdes: String) -> void:
    var id := str(component.id)
    var metal := _material(METAL_COLOR, 0.92, 0.16)
    var copper := _material(COPPER_COLOR, 0.78, 0.24)
    var black := _material(DARK_PACKAGE, 0.08, 0.32)
    var tan := _material(Color(0.70, 0.53, 0.28, 1.0), 0.05, 0.55)

    match id:
        "resistor":
            _build_resistor(body, tan, metal)
        "potentiometer":
            _build_potentiometer(body, metal)
        "ceramic_cap":
            _build_ceramic_cap(body, metal)
        "electrolytic_cap":
            _build_electrolytic_cap(body, metal)
        "inductor":
            _build_inductor(body, copper, black, metal)
        "diode":
            _build_axial_diode(body, black, metal, false)
        "zener":
            _build_axial_diode(body, _material(Color(0.13, 0.19, 0.23, 1.0), 0.10, 0.28), metal, true)
        "led":
            _build_led_component(body, metal)
        "npn", "pnp":
            _build_to92(body, black, metal)
        "nmos", "pmos":
            _build_to220(body, black, metal)
        "logic":
            _build_dip(body, 7, black, metal)
        "opamp", "comparator":
            _build_dip(body, 4, black, metal)
        "regulator":
            _build_regulator(body, black, metal)
        "fuse":
            _build_fuse(body, metal)
        "switch":
            _build_switch(body, black, metal)
        "relay":
            _build_relay(body, metal)
        "connector":
            _build_connector(body, black, metal)
        "test_point":
            _build_test_point(body, metal)
        "sensor":
            _build_sensor(body, metal)
        "buzzer":
            _build_buzzer(body, black, metal)
        "power":
            _build_power_entry(body, metal)
        "ground":
            _build_ground(body, metal)
        _:
            _box_child(body, "Package3D", Vector3(0.62, 0.34, 0.46), Vector3(0, 0.23, 0), black)
            _axial_leads(body, metal)

func _build_resistor(body: Node3D, body_material: Material, metal: Material) -> void:
    var core := _cylinder_child(body, "AxialResistorBody3D", 0.15, 0.48, Vector3(0, 0.24, 0), body_material)
    core.rotation_degrees.z = 90.0
    var band_colors := [
        Color(0.36, 0.08, 0.04, 1.0),
        Color(0.06, 0.06, 0.06, 1.0),
        Color(0.76, 0.12, 0.06, 1.0),
        Color(0.78, 0.58, 0.12, 1.0)
    ]
    var xs := [-0.13, -0.045, 0.045, 0.13]
    for i in range(xs.size()):
        var band := _cylinder_child(body, "Band%d" % i, 0.156, 0.034, Vector3(xs[i], 0.24, 0), _material(band_colors[i], 0.18, 0.42))
        band.rotation_degrees.z = 90.0
    _axial_leads(body, metal)

func _build_potentiometer(body: Node3D, metal: Material) -> void:
    _box_child(body, "PotBody3D", Vector3(0.58, 0.32, 0.48), Vector3(0, 0.23, 0), _material(Color(0.05, 0.20, 0.34, 1.0), 0.10, 0.34))
    _cylinder_child(body, "PotShaft3D", 0.105, 0.30, Vector3(0, 0.53, 0), metal)
    _cylinder_child(body, "PotKnob3D", 0.17, 0.13, Vector3(0, 0.73, 0), _material(Color(0.10, 0.11, 0.12, 1.0), 0.05, 0.50))
    for x in [-0.20, 0.0, 0.20]:
        _box_child(body, "PotLead3D", Vector3(0.045, 0.22, 0.055), Vector3(x, 0.05, 0.0), metal)

func _build_ceramic_cap(body: Node3D, metal: Material) -> void:
    var disc := _cylinder_child(body, "CeramicDisc3D", 0.25, 0.10, Vector3(0, 0.34, 0), _material(Color(0.88, 0.45, 0.12, 1.0), 0.02, 0.60))
    disc.rotation_degrees.x = 90.0
    for x in [-0.14, 0.14]:
        _box_child(body, "CapLead3D", Vector3(0.045, 0.36, 0.045), Vector3(x, 0.12, 0), metal)

func _build_electrolytic_cap(body: Node3D, metal: Material) -> void:
    var can_mat := _material(Color(0.06, 0.09, 0.12, 1.0), 0.44, 0.24)
    _cylinder_child(body, "ElectrolyticCan3D", 0.25, 0.56, Vector3(0, 0.36, 0), can_mat)
    _cylinder_child(body, "ElectrolyticTop3D", 0.245, 0.025, Vector3(0, 0.65, 0), metal)
    _box_child(body, "PolarityStripe3D", Vector3(0.045, 0.48, 0.30), Vector3(0.23, 0.36, 0), _material(Color(0.82, 0.84, 0.85, 1.0), 0.12, 0.48))
    for x in [-0.10, 0.10]:
        _box_child(body, "CapLead3D", Vector3(0.04, 0.16, 0.04), Vector3(x, 0.05, 0), metal)

func _build_inductor(body: Node3D, copper: Material, black: Material, metal: Material) -> void:
    _cylinder_child(body, "InductorCore3D", 0.27, 0.30, Vector3(0, 0.25, 0), black)
    for i in range(4):
        _cylinder_child(body, "InductorCopperLayer%d" % i, 0.285 - i * 0.018, 0.035, Vector3(0, 0.15 + i * 0.065, 0), copper)
    for x in [-0.18, 0.18]:
        _box_child(body, "InductorLead3D", Vector3(0.04, 0.15, 0.04), Vector3(x, 0.05, 0), metal)

func _build_axial_diode(body: Node3D, package_material: Material, metal: Material, zener: bool) -> void:
    var glass := package_material
    if not zener:
        glass = _glass_material(Color(0.88, 0.56, 0.18, 0.52))
    var package := _cylinder_child(body, "DiodeBody3D", 0.13, 0.44, Vector3(0, 0.23, 0), glass)
    package.rotation_degrees.z = 90.0
    var band := _cylinder_child(body, "CathodeBand3D", 0.137, 0.055, Vector3(0.12, 0.23, 0), _material(Color(0.84, 0.86, 0.88, 1.0), 0.65, 0.20))
    band.rotation_degrees.z = 90.0
    _axial_leads(body, metal)

func _build_led_component(body: Node3D, metal: Material) -> void:
    _cylinder_child(body, "LedFlange3D", 0.25, 0.09, Vector3(0, 0.22, 0), _glass_material(Color(0.82, 0.08, 0.06, 0.60)))
    _sphere_child(body, "LedLens3D", 0.225, Vector3(0, 0.41, 0), _glass_material(Color(0.95, 0.10, 0.07, 0.62)), Vector3(1.0, 1.18, 1.0))
    _box_child(body, "LedAnode3D", Vector3(0.045, 0.24, 0.045), Vector3(-0.09, 0.06, 0), metal)
    _box_child(body, "LedCathode3D", Vector3(0.055, 0.20, 0.055), Vector3(0.09, 0.08, 0), metal)

func _build_to92(body: Node3D, package_material: Material, metal: Material) -> void:
    _sphere_child(body, "TO92Body3D", 0.25, Vector3(0, 0.34, 0), package_material, Vector3(1.0, 1.05, 0.68))
    _box_child(body, "TO92Flat3D", Vector3(0.50, 0.38, 0.08), Vector3(0, 0.34, 0.17), package_material)
    for x in [-0.18, 0.0, 0.18]:
        _box_child(body, "TO92Lead3D", Vector3(0.035, 0.26, 0.035), Vector3(x, 0.08, 0), metal)

func _build_to220(body: Node3D, package_material: Material, metal: Material) -> void:
    _box_child(body, "TO220Body3D", Vector3(0.52, 0.46, 0.16), Vector3(0, 0.32, 0), package_material)
    _box_child(body, "TO220Tab3D", Vector3(0.46, 0.22, 0.06), Vector3(0, 0.64, 0), metal)
    _cylinder_child(body, "TO220Hole3D", 0.07, 0.065, Vector3(0, 0.66, 0), _material(Color(0.015, 0.018, 0.02, 1.0), 0.0, 0.9))
    for x in [-0.17, 0.0, 0.17]:
        _box_child(body, "TO220Lead3D", Vector3(0.035, 0.25, 0.035), Vector3(x, 0.07, 0), metal)

func _build_dip(body: Node3D, pins_per_side: int, package_material: Material, metal: Material) -> void:
    var length := 0.66 if pins_per_side <= 4 else 0.80
    _box_child(body, "DIPPackage3D", Vector3(length, 0.24, 0.46), Vector3(0, 0.24, 0), package_material)
    var denom := maxf(1.0, float(pins_per_side - 1))
    for i in range(pins_per_side):
        var x := -length * 0.42 + (length * 0.84) * float(i) / denom
        _box_child(body, "DIPPin3D", Vector3(0.026, 0.06, 0.15), Vector3(x, 0.11, 0.29), metal)
        _box_child(body, "DIPPin3D", Vector3(0.026, 0.06, 0.15), Vector3(x, 0.11, -0.29), metal)

func _build_regulator(body: Node3D, package_material: Material, metal: Material) -> void:
    _box_child(body, "RegulatorBody3D", Vector3(0.48, 0.30, 0.20), Vector3(0, 0.25, 0), package_material)
    _box_child(body, "RegulatorTab3D", Vector3(0.52, 0.07, 0.34), Vector3(0, 0.16, -0.20), metal)
    for x in [-0.17, 0.0, 0.17]:
        _box_child(body, "RegulatorLead3D", Vector3(0.035, 0.22, 0.035), Vector3(x, 0.07, 0.14), metal)

func _build_fuse(body: Node3D, metal: Material) -> void:
    var tube := _cylinder_child(body, "FuseGlass3D", 0.13, 0.48, Vector3(0, 0.23, 0), _glass_material(GLASS_COLOR))
    tube.rotation_degrees.z = 90.0
    for x in [-0.25, 0.25]:
        var cap := _cylinder_child(body, "FuseCap3D", 0.145, 0.11, Vector3(x, 0.23, 0), metal)
        cap.rotation_degrees.z = 90.0
    _box_child(body, "FuseElement3D", Vector3(0.42, 0.018, 0.018), Vector3(0, 0.23, 0), _material(Color(0.72, 0.55, 0.24, 1.0), 0.74, 0.18))
    _axial_leads(body, metal, 0.35)

func _build_switch(body: Node3D, package_material: Material, metal: Material) -> void:
    _box_child(body, "SwitchBase3D", Vector3(0.58, 0.20, 0.42), Vector3(0, 0.18, 0), package_material)
    var lever := _box_child(body, "SwitchLever3D", Vector3(0.08, 0.38, 0.08), Vector3(0.08, 0.43, 0), metal)
    lever.rotation_degrees.z = -28.0
    for x in [-0.18, 0.18]:
        _box_child(body, "SwitchLead3D", Vector3(0.045, 0.18, 0.045), Vector3(x, 0.05, 0), metal)

func _build_relay(body: Node3D, metal: Material) -> void:
    _box_child(body, "RelayCase3D", Vector3(0.66, 0.46, 0.54), Vector3(0, 0.30, 0), _material(Color(0.06, 0.14, 0.24, 1.0), 0.08, 0.30))
    for x in [-0.25, -0.08, 0.08, 0.25]:
        _box_child(body, "RelayPin3D", Vector3(0.035, 0.16, 0.035), Vector3(x, 0.05, 0.22), metal)
        _box_child(body, "RelayPin3D", Vector3(0.035, 0.16, 0.035), Vector3(x, 0.05, -0.22), metal)

func _build_connector(body: Node3D, package_material: Material, metal: Material) -> void:
    _box_child(body, "HeaderBody3D", Vector3(0.72, 0.18, 0.30), Vector3(0, 0.17, 0), package_material)
    for x in [-0.27, -0.09, 0.09, 0.27]:
        _cylinder_child(body, "HeaderPin3D", 0.026, 0.40, Vector3(x, 0.32, 0), metal)

func _build_test_point(body: Node3D, metal: Material) -> void:
    _cylinder_child(body, "TestPointPad3D", 0.20, 0.035, Vector3(0, 0.04, 0), metal)
    _cylinder_child(body, "TestPointPost3D", 0.055, 0.38, Vector3(0, 0.24, 0), metal)
    var ring := _cylinder_child(body, "TestPointRing3D", 0.13, 0.055, Vector3(0, 0.43, 0), metal)
    ring.scale = Vector3(1.0, 1.0, 0.42)

func _build_sensor(body: Node3D, metal: Material) -> void:
    _cylinder_child(body, "SensorCan3D", 0.24, 0.36, Vector3(0, 0.28, 0), _material(Color(0.50, 0.56, 0.58, 1.0), 0.78, 0.22))
    _cylinder_child(body, "SensorWindow3D", 0.13, 0.025, Vector3(0, 0.47, 0), _glass_material(Color(0.08, 0.18, 0.24, 0.66)))
    for x in [-0.15, 0.0, 0.15]:
        _box_child(body, "SensorLead3D", Vector3(0.035, 0.18, 0.035), Vector3(x, 0.06, 0), metal)

func _build_buzzer(body: Node3D, package_material: Material, metal: Material) -> void:
    _cylinder_child(body, "BuzzerCase3D", 0.29, 0.30, Vector3(0, 0.23, 0), package_material)
    _cylinder_child(body, "BuzzerTop3D", 0.28, 0.025, Vector3(0, 0.39, 0), _material(Color(0.08, 0.09, 0.10, 1.0), 0.05, 0.58))
    _cylinder_child(body, "BuzzerPort3D", 0.055, 0.03, Vector3(0, 0.41, 0), _material(Color(0.005, 0.006, 0.007, 1.0), 0.0, 1.0))
    for x in [-0.11, 0.11]:
        _box_child(body, "BuzzerLead3D", Vector3(0.04, 0.18, 0.04), Vector3(x, 0.06, 0), metal)

func _build_power_entry(body: Node3D, metal: Material) -> void:
    _box_child(body, "PowerTerminalBlock3D", Vector3(0.66, 0.42, 0.48), Vector3(0, 0.28, 0), _material(Color(0.08, 0.30, 0.20, 1.0), 0.08, 0.40))
    for x in [-0.18, 0.18]:
        _cylinder_child(body, "PowerScrew3D", 0.085, 0.05, Vector3(x, 0.52, 0), metal)
        _box_child(body, "PowerLead3D", Vector3(0.05, 0.18, 0.05), Vector3(x, 0.07, 0), metal)

func _build_ground(body: Node3D, metal: Material) -> void:
    _cylinder_child(body, "GroundPad3D", 0.28, 0.045, Vector3(0, 0.04, 0), metal)
    _cylinder_child(body, "GroundStud3D", 0.065, 0.32, Vector3(0, 0.22, 0), metal)
    _cylinder_child(body, "GroundWasher3D", 0.16, 0.04, Vector3(0, 0.40, 0), metal)

func _axial_leads(body: Node3D, metal: Material, body_half_width := 0.24) -> void:
    var lead_length := 0.28
    _box_child(body, "LeadL3D", Vector3(lead_length, 0.035, 0.035), Vector3(-(body_half_width + lead_length * 0.5), 0.23, 0), metal)
    _box_child(body, "LeadR3D", Vector3(lead_length, 0.035, 0.035), Vector3(body_half_width + lead_length * 0.5, 0.23, 0), metal)

func _sphere_child(parent: Node3D, node_name: String, radius: float, position_value: Vector3, material: Material, scale_value := Vector3.ONE) -> MeshInstance3D:
    var node := MeshInstance3D.new()
    node.name = node_name
    var mesh := SphereMesh.new()
    mesh.radius = radius
    mesh.height = radius * 2.0
    mesh.radial_segments = 32
    mesh.rings = 16
    node.mesh = mesh
    node.position = position_value
    node.scale = scale_value
    node.material_override = material
    parent.add_child(node)
    return node

func _glass_material(color: Color) -> StandardMaterial3D:
    var material := _material(color, 0.04, 0.12)
    material.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
    return material

func debug_board_is_visual_canvas() -> bool:
    return find_children("*", "Label3D", true, false).is_empty() and find_children("SolderPad*", "MeshInstance3D", true, false).is_empty()

func debug_vector_schematic_ready() -> bool:
    return schematic_canvas != null

func debug_component_visual_has_text(body: StaticBody3D) -> bool:
    if body == null:
        return false
    return not body.find_children("*", "Label3D", true, false).is_empty()
