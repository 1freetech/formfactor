extends "res://scripts/interactive_lab_1126.gd"

# FormFactor 1.127 // PIN GRAPH
# The editor now records exact component-pin endpoints. Godot owns interaction
# and presentation; the C++ engineering core remains authoritative.

const PIN_GRAPH_1127 = preload("res://scripts/pin_net_graph_1127.gd")
const ENGINEERING_BRIDGE_1127 = preload("res://scripts/engineering_bridge_1127.gd")
const SCHEMATIC_CANVAS_1127 = preload("res://scripts/schematic_symbol_canvas_1127.gd")
const BLUEPRINT_PATH := "user://engineering_blueprints/latest_blueprint_1_127.json"
const ENGINEERING_SNAPSHOT_PATH := "user://last_engineering_board_1_127.json"

enum EditorTool {
    SELECT,
    PLACE,
    WIRE,
    MOVE,
    ROTATE,
    INSPECT,
    TEST
}

const PIN_LIBRARY := {
    "resistor": [
        {"id":"1","label":"1","type":"passive"},
        {"id":"2","label":"2","type":"passive"}
    ],
    "potentiometer": [
        {"id":"1","label":"1","type":"passive"},
        {"id":"W","label":"W","type":"passive"},
        {"id":"2","label":"2","type":"passive"}
    ],
    "ceramic_cap": [
        {"id":"1","label":"1","type":"passive"},
        {"id":"2","label":"2","type":"passive"}
    ],
    "electrolytic_cap": [
        {"id":"POS","label":"+","type":"passive"},
        {"id":"NEG","label":"-","type":"passive"}
    ],
    "inductor": [
        {"id":"1","label":"1","type":"passive"},
        {"id":"2","label":"2","type":"passive"}
    ],
    "diode": [
        {"id":"A","label":"A","type":"passive"},
        {"id":"K","label":"K","type":"passive"}
    ],
    "zener": [
        {"id":"A","label":"A","type":"passive"},
        {"id":"K","label":"K","type":"passive"}
    ],
    "led": [
        {"id":"A","label":"A","type":"passive"},
        {"id":"K","label":"K","type":"passive"}
    ],
    "npn": [
        {"id":"B","label":"B","type":"passive"},
        {"id":"C","label":"C","type":"passive"},
        {"id":"E","label":"E","type":"passive"}
    ],
    "pnp": [
        {"id":"B","label":"B","type":"passive"},
        {"id":"C","label":"C","type":"passive"},
        {"id":"E","label":"E","type":"passive"}
    ],
    "nmos": [
        {"id":"G","label":"G","type":"passive"},
        {"id":"D","label":"D","type":"passive"},
        {"id":"S","label":"S","type":"passive"}
    ],
    "pmos": [
        {"id":"G","label":"G","type":"passive"},
        {"id":"D","label":"D","type":"passive"},
        {"id":"S","label":"S","type":"passive"}
    ],
    "logic": [
        {"id":"A","label":"A","type":"digital_input"},
        {"id":"B","label":"B","type":"digital_input"},
        {"id":"Y","label":"Y","type":"digital_output"}
    ],
    "opamp": [
        {"id":"INP","label":"+IN","type":"passive"},
        {"id":"INN","label":"-IN","type":"passive"},
        {"id":"OUT","label":"OUT","type":"passive"},
        {"id":"VP","label":"V+","type":"power_input"},
        {"id":"VN","label":"V-","type":"power_input"}
    ],
    "comparator": [
        {"id":"INP","label":"+IN","type":"passive"},
        {"id":"INN","label":"-IN","type":"passive"},
        {"id":"OUT","label":"OUT","type":"digital_output"},
        {"id":"VP","label":"V+","type":"power_input"},
        {"id":"VN","label":"V-","type":"power_input"}
    ],
    "regulator": [
        {"id":"VIN","label":"VIN","type":"power_input"},
        {"id":"GND","label":"GND","type":"passive"},
        {"id":"VOUT","label":"VOUT","type":"power_output"}
    ],
    "fuse": [
        {"id":"1","label":"1","type":"passive"},
        {"id":"2","label":"2","type":"passive"}
    ],
    "switch": [
        {"id":"1","label":"1","type":"passive"},
        {"id":"2","label":"2","type":"passive"}
    ],
    "relay": [
        {"id":"COILP","label":"COIL+","type":"passive"},
        {"id":"COILN","label":"COIL-","type":"passive"},
        {"id":"COM","label":"COM","type":"passive"},
        {"id":"NO","label":"NO","type":"passive"},
        {"id":"NC","label":"NC","type":"passive"}
    ],
    "connector": [
        {"id":"1","label":"1","type":"passive"},
        {"id":"2","label":"2","type":"passive"},
        {"id":"3","label":"3","type":"passive"},
        {"id":"4","label":"4","type":"passive"}
    ],
    "test_point": [
        {"id":"1","label":"TP","type":"passive"}
    ],
    "sensor": [
        {"id":"VCC","label":"VCC","type":"power_input"},
        {"id":"GND","label":"GND","type":"passive"},
        {"id":"OUT","label":"OUT","type":"digital_output"}
    ],
    "buzzer": [
        {"id":"POS","label":"+","type":"power_input"},
        {"id":"NEG","label":"-","type":"passive"}
    ],
    "power": [
        {"id":"POS","label":"+","type":"power_output"},
        {"id":"NEG","label":"-","type":"passive"}
    ],
    "ground": [
        {"id":"1","label":"GND","type":"passive"}
    ]
}

# These are visual/catalogue links, not claims that every generic game part has
# this exact package. The examples named from embeddedalpha/PCB-Design are MIT
# reference data and remain marked reference-only until component provenance is
# verified by the engineering catalogue.
const PACKAGE_LINKS := {
    "resistor": {"symbol":"Device:R","footprint":"Resistor_SMD:R_0402_1005Metric","package":"0402 reference / axial visual","status":"reference-only"},
    "ceramic_cap": {"symbol":"Device:C_Small","footprint":"Capacitor_SMD:C_0201_0603Metric","package":"0201 reference / ceramic visual","status":"reference-only"},
    "connector": {"symbol":"Connector_Generic:Conn_01x04","footprint":"Connector_PinHeader_2.54mm:*","package":"1x4 header family","status":"reference-only"},
    "diode": {"symbol":"Device:D","footprint":"catalog-required","package":"axial visual","status":"visual-only"},
    "zener": {"symbol":"Device:D_Zener","footprint":"catalog-required","package":"axial visual","status":"visual-only"},
    "led": {"symbol":"Device:LED","footprint":"catalog-required","package":"through-hole LED visual","status":"visual-only"},
    "logic": {"symbol":"74xx:*","footprint":"catalog-required","package":"DIP visual","status":"visual-only"},
    "opamp": {"symbol":"Amplifier_Operational:*","footprint":"catalog-required","package":"DIP visual","status":"visual-only"},
    "comparator": {"symbol":"Comparator:*","footprint":"catalog-required","package":"DIP visual","status":"visual-only"},
    "regulator": {"symbol":"Regulator_*:*","footprint":"catalog-required","package":"regulator visual","status":"visual-only"},
    "power": {"symbol":"power:*","footprint":"catalog-required","package":"terminal-block visual","status":"visual-only"},
    "ground": {"symbol":"power:GND","footprint":"none","package":"reference node","status":"symbol-only"}
}

const INTERNAL_TOPOLOGY_PAIRS := {
    "resistor": [["1", "2"]],
    "inductor": [["1", "2"]],
    "fuse": [["1", "2"]]
}

var editor_tool: EditorTool = EditorTool.SELECT
var wire_start_pin := ""
var pin_catalog_valid := false
var pin_catalog_errors: Array[String] = []
var tool_mode_label: Label
var pin_detail_label: Label
var truth_panel: ColorRect
var truth_gate_selector: OptionButton
var truth_table_label: Label
var last_pin_precheck_passed := false

func _ready() -> void:
    super._ready()
    pin_catalog_errors = ENGINEERING_BRIDGE_1127.validate_pin_catalog(PIN_LIBRARY)
    pin_catalog_valid = pin_catalog_errors.is_empty()
    _ensure_all_pin_ports()
    _set_editor_tool(EditorTool.SELECT)
    _export_engineering_snapshot()

    if status_label != null:
        status_label.text = "1.127 ready. Parts now have named pins. WIRE connects exact pin endpoints."
    if build_status != null:
        build_status.text = "PIN GRAPH READY" if pin_catalog_valid else "PIN CATALOG ERROR"
    if not pin_catalog_valid:
        push_error("FormFactor 1.127 pin catalog invalid: %s" % "; ".join(pin_catalog_errors))

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.126"):
                label.text = "FORMFACTOR 1.127 // PIN GRAPH"
    var version_label := get_node_or_null("WorkbenchVersion3D") as Label3D
    if version_label != null:
        version_label.text = "1.127 // PIN GRAPH"

func _build_gameplay_hud() -> void:
    super._build_gameplay_hud()
    var hud := get_node_or_null("HUD") as CanvasLayer
    if hud == null:
        return

    var toolbar := ColorRect.new()
    toolbar.name = "PinGraphToolbar1127"
    toolbar.position = Vector2(404, 18)
    toolbar.size = Vector2(674, 78)
    toolbar.color = Color(0.008, 0.018, 0.024, 0.94)
    hud.add_child(toolbar)

    tool_mode_label = Label.new()
    tool_mode_label.name = "ToolMode1127"
    tool_mode_label.position = Vector2(10, 6)
    tool_mode_label.size = Vector2(654, 18)
    tool_mode_label.add_theme_color_override("font_color", NEON)
    tool_mode_label.add_theme_font_size_override("font_size", 11)
    toolbar.add_child(tool_mode_label)

    _tool_button(toolbar, "SELECT", Vector2(10, 28), 72, func() -> void: _set_editor_tool(EditorTool.SELECT))
    _tool_button(toolbar, "WIRE", Vector2(86, 28), 62, _toggle_wire_mode)
    _tool_button(toolbar, "MOVE", Vector2(152, 28), 62, func() -> void: _set_editor_tool(EditorTool.MOVE))
    _tool_button(toolbar, "ROTATE", Vector2(218, 28), 72, func() -> void:
        _set_editor_tool(EditorTool.ROTATE)
        rotate_active_component(90.0)
    )
    _tool_button(toolbar, "INSPECT", Vector2(294, 28), 78, func() -> void: _set_editor_tool(EditorTool.INSPECT))
    _tool_button(toolbar, "TEST", Vector2(376, 28), 58, _test_player_circuit)
    _tool_button(toolbar, "TRUTH", Vector2(438, 28), 62, _toggle_truth_panel)
    _tool_button(toolbar, "SAVE BP", Vector2(504, 28), 72, _save_blueprint)
    _tool_button(toolbar, "LOAD BP", Vector2(580, 28), 78, _load_blueprint)

    pin_detail_label = Label.new()
    pin_detail_label.name = "PinDetail1127"
    pin_detail_label.position = Vector2(404, 98)
    pin_detail_label.size = Vector2(674, 20)
    pin_detail_label.add_theme_color_override("font_color", Color(0.74, 0.84, 0.86, 1.0))
    pin_detail_label.add_theme_font_size_override("font_size", 10)
    pin_detail_label.text = "PIN INFO // click a part to inspect its named pins"
    hud.add_child(pin_detail_label)

    _build_truth_panel(hud)

func _tool_button(parent: Control, text_value: String, pos: Vector2, width: float, callback: Callable) -> Button:
    var button := Button.new()
    button.text = text_value
    button.position = pos
    button.size = Vector2(width, 36)
    button.add_theme_font_size_override("font_size", 10)
    button.pressed.connect(callback)
    parent.add_child(button)
    return button

func _build_truth_panel(hud: CanvasLayer) -> void:
    truth_panel = ColorRect.new()
    truth_panel.name = "TruthTablePanel1127"
    truth_panel.position = Vector2(760, 158)
    truth_panel.size = Vector2(314, 332)
    truth_panel.color = Color(0.006, 0.014, 0.020, 0.985)
    truth_panel.visible = false
    hud.add_child(truth_panel)

    var title := Label.new()
    title.position = Vector2(12, 10)
    title.text = "DIGITAL TRUTH TABLE // PREVIEW"
    title.add_theme_color_override("font_color", NEON)
    title.add_theme_font_size_override("font_size", 13)
    truth_panel.add_child(title)

    truth_gate_selector = OptionButton.new()
    truth_gate_selector.position = Vector2(12, 40)
    truth_gate_selector.size = Vector2(180, 34)
    for gate_name in ["NOT", "AND", "OR", "NAND", "NOR", "XOR", "XNOR"]:
        truth_gate_selector.add_item(gate_name)
    truth_gate_selector.item_selected.connect(func(_index: int) -> void: _refresh_truth_table_preview())
    truth_panel.add_child(truth_gate_selector)

    var close := Button.new()
    close.text = "CLOSE"
    close.position = Vector2(220, 40)
    close.size = Vector2(80, 34)
    close.pressed.connect(_toggle_truth_panel)
    truth_panel.add_child(close)

    truth_table_label = Label.new()
    truth_table_label.position = Vector2(12, 86)
    truth_table_label.size = Vector2(288, 230)
    truth_table_label.add_theme_font_size_override("font_size", 12)
    truth_table_label.add_theme_color_override("font_color", Color(0.84, 0.91, 0.92, 1.0))
    truth_panel.add_child(truth_table_label)
    _refresh_truth_table_preview()

func _toggle_truth_panel() -> void:
    if truth_panel == null:
        return
    truth_panel.visible = not truth_panel.visible
    if truth_panel.visible:
        _refresh_truth_table_preview()

func _truth_eval_preview(gate_name: String, a: int, b: int) -> int:
    match gate_name:
        "NOT":
            return 0 if a == 1 else 1
        "AND":
            return 1 if a == 1 and b == 1 else 0
        "OR":
            return 1 if a == 1 or b == 1 else 0
        "NAND":
            return 0 if a == 1 and b == 1 else 1
        "NOR":
            return 0 if a == 1 or b == 1 else 1
        "XOR":
            return 1 if a != b else 0
        "XNOR":
            return 1 if a == b else 0
    return 0

func _refresh_truth_table_preview() -> void:
    if truth_gate_selector == null or truth_table_label == null:
        return
    var gate_name := truth_gate_selector.get_item_text(truth_gate_selector.selected)
    var lines: Array[String] = []
    lines.append("Gate: %s" % gate_name)
    lines.append("")
    if gate_name == "NOT":
        lines.append(" A | Y")
        lines.append("---+---")
        for a in [0, 1]:
            lines.append(" %d | %d" % [a, _truth_eval_preview(gate_name, a, 0)])
    else:
        lines.append(" A B | Y")
        lines.append("-----+---")
        for a in [0, 1]:
            for b in [0, 1]:
                lines.append(" %d %d | %d" % [a, b, _truth_eval_preview(gate_name, a, b)])
    lines.append("")
    lines.append("Preview only.")
    lines.append("C++ event simulation is authoritative.")
    truth_table_label.text = "\n".join(lines)

func _install_vector_schematic() -> void:
    var hud := get_node_or_null("HUD")
    if hud == null:
        return
    var panel := hud.get_node_or_null("SchematicMirrorPanel") as ColorRect
    if panel == null:
        return
    if schematic_label != null:
        schematic_label.visible = false
    for child in panel.get_children():
        if child is Control and str(child.name).begins_with("VectorSchematicCanvas"):
            child.queue_free()
    schematic_canvas = SCHEMATIC_CANVAS_1127.new()
    schematic_canvas.name = "VectorSchematicCanvas1127"
    schematic_canvas.position = Vector2(8, 38)
    schematic_canvas.size = Vector2(352, 364)
    schematic_canvas.mouse_filter = Control.MOUSE_FILTER_IGNORE
    panel.add_child(schematic_canvas)

func select_component(component_id: String) -> void:
    super.select_component(component_id)
    if selected_component_id == component_id:
        _set_editor_tool(EditorTool.PLACE, false)

func _set_editor_tool(tool: EditorTool, clear_selection := true) -> void:
    editor_tool = tool
    wire_mode = tool == EditorTool.WIRE
    if tool != EditorTool.WIRE:
        wire_start = null
        wire_start_pin = ""
    if clear_selection and tool != EditorTool.PLACE:
        selected_component_id = ""
    _update_selected_label()
    if tool_mode_label != null:
        tool_mode_label.text = "TOOL // %s" % EditorTool.keys()[int(editor_tool)]

func _toggle_wire_mode() -> void:
    if editor_tool == EditorTool.WIRE:
        _set_editor_tool(EditorTool.SELECT)
        if build_status != null:
            build_status.text = "WIRE TOOL OFF"
    else:
        _set_editor_tool(EditorTool.WIRE)
        if build_status != null:
            build_status.text = "WIRE TOOL // click exact package pins"

func _handle_world_click(screen_pos: Vector2) -> void:
    if editor_tool == EditorTool.INSPECT and not wire_mode and selected_component_id.is_empty():
        var collider := _ray_collider(screen_pos)
        if collider != null and bool(collider.get_meta("placed_component", false)):
            active_component = collider as StaticBody3D
            _show_pin_details(active_component)
            return
    super._handle_world_click(screen_pos)
    if active_component != null and is_instance_valid(active_component):
        _show_pin_details(active_component)

func _place_component_at_world(world_pos: Vector3) -> StaticBody3D:
    var body := super._place_component_at_world(world_pos)
    if body == null:
        return null
    _ensure_pin_ports(body)
    _apply_engineering_link_metadata(body)
    _refresh_schematic()
    if not history_replaying:
        _export_engineering_snapshot()
    _set_editor_tool(EditorTool.SELECT)
    return body

func _ensure_all_pin_ports() -> void:
    for body in placed_components:
        if body != null and is_instance_valid(body):
            _ensure_pin_ports(body)
            _apply_engineering_link_metadata(body)

func _ensure_pin_ports(body: StaticBody3D) -> void:
    if body == null or not is_instance_valid(body):
        return
    var component_id := str(body.get_meta("component_id", ""))
    var pins: Array = PIN_LIBRARY.get(component_id, [])
    var existing := body.get_node_or_null("PinPorts1127")
    if existing != null:
        existing.queue_free()

    var root := Node3D.new()
    root.name = "PinPorts1127"
    body.add_child(root)

    var material := StandardMaterial3D.new()
    material.albedo_color = Color(0.10, 0.86, 1.0, 0.92)
    material.emission_enabled = true
    material.emission = Color(0.10, 0.86, 1.0, 1.0)
    material.emission_energy_multiplier = 1.8
    material.roughness = 0.20

    for index in range(pins.size()):
        var pin := pins[index] as Dictionary
        var marker := MeshInstance3D.new()
        marker.name = "Pin_%s" % str(pin.get("id", index + 1))
        marker.set_meta("pin_id", str(pin.get("id", "")))
        marker.set_meta("pin_label", str(pin.get("label", pin.get("id", ""))))
        marker.set_meta("pin_type", str(pin.get("type", "passive")))
        var mesh := CylinderMesh.new()
        mesh.top_radius = 0.060
        mesh.bottom_radius = 0.060
        mesh.height = 0.026
        mesh.radial_segments = 18
        marker.mesh = mesh
        marker.position = _pin_local_position(index, pins.size())
        marker.material_override = material
        root.add_child(marker)

func _pin_local_position(index: int, count: int) -> Vector3:
    if count <= 1:
        return Vector3(0.0, 0.60, 0.0)
    if count == 2:
        return Vector3(-0.48 if index == 0 else 0.48, 0.24, 0.0)
    if count == 3:
        var x3 := [-0.40, 0.0, 0.40]
        return Vector3(x3[index], 0.20, 0.32)
    if count == 4:
        var p4 := [
            Vector3(-0.34, 0.20, -0.32),
            Vector3(0.34, 0.20, -0.32),
            Vector3(-0.34, 0.20, 0.32),
            Vector3(0.34, 0.20, 0.32)
        ]
        return p4[index]
    var side_count := ceili(float(count) / 2.0)
    if index < side_count:
        var z_left := -0.32 + 0.64 * float(index) / maxf(1.0, float(side_count - 1))
        return Vector3(-0.42, 0.20, z_left)
    var right_index := index - side_count
    var right_count := count - side_count
    var z_right := 0.32 - 0.64 * float(right_index) / maxf(1.0, float(right_count - 1))
    return Vector3(0.42, 0.20, z_right)

func _apply_engineering_link_metadata(body: StaticBody3D) -> void:
    var component_id := str(body.get_meta("component_id", ""))
    var link: Dictionary = PACKAGE_LINKS.get(component_id, {
        "symbol":"catalog-required",
        "footprint":"catalog-required",
        "package":"visual-only",
        "status":"unverified"
    })
    body.set_meta("symbol_id", str(link.get("symbol", "catalog-required")))
    body.set_meta("footprint_id", str(link.get("footprint", "catalog-required")))
    body.set_meta("package_style", str(link.get("package", "visual-only")))
    body.set_meta("engineering_link_status", str(link.get("status", "unverified")))

func _show_pin_details(body: StaticBody3D) -> void:
    if pin_detail_label == null or body == null or not is_instance_valid(body):
        return
    var component_id := str(body.get_meta("component_id", ""))
    var refdes := str(body.get_meta("refdes", "?"))
    var pins: Array = PIN_LIBRARY.get(component_id, [])
    var labels: Array[String] = []
    for pin_variant in pins:
        var pin := pin_variant as Dictionary
        labels.append(str(pin.get("label", pin.get("id", "?"))))
    pin_detail_label.text = "%s PINS // %s   SYMBOL %s   FOOTPRINT %s" % [
        refdes,
        ", ".join(labels),
        str(body.get_meta("symbol_id", "catalog-required")),
        str(body.get_meta("footprint_id", "catalog-required"))
    ]

func _pin_hit_for_screen(screen_pos: Vector2) -> Dictionary:
    var origin := camera.project_ray_origin(screen_pos)
    var ray_end := origin + camera.project_ray_normal(screen_pos) * 100.0
    var query := PhysicsRayQueryParameters3D.create(origin, ray_end)
    query.collide_with_bodies = true
    var hit := get_world_3d().direct_space_state.intersect_ray(query)
    if hit.is_empty():
        return {}
    var body := hit.get("collider") as StaticBody3D
    if body == null or not bool(body.get_meta("placed_component", false)):
        return {}
    _ensure_pin_ports(body)
    var pin_root := body.get_node_or_null("PinPorts1127") as Node3D
    if pin_root == null:
        return {}

    var hit_position: Vector3 = hit.get("position", body.global_position)
    var best: MeshInstance3D = null
    var best_distance := INF
    for child in pin_root.get_children():
        var marker := child as MeshInstance3D
        if marker == null:
            continue
        var distance := marker.global_position.distance_to(hit_position)
        if distance < best_distance:
            best_distance = distance
            best = marker
    if best == null:
        return {}
    return {
        "body": body,
        "pin": str(best.get_meta("pin_id", "")),
        "label": str(best.get_meta("pin_label", best.get_meta("pin_id", "")))
    }

func _wire_click(screen_pos: Vector2) -> void:
    var endpoint := _pin_hit_for_screen(screen_pos)
    if endpoint.is_empty():
        if status_label != null:
            status_label.text = "WIRE: click a placed component near the pin you want."
        return

    var body := endpoint.get("body") as StaticBody3D
    var pin_id := str(endpoint.get("pin", ""))
    var pin_label := str(endpoint.get("label", pin_id))
    if body == null or pin_id.is_empty():
        return

    if wire_start == null:
        wire_start = body
        wire_start_pin = pin_id
        if status_label != null:
            status_label.text = "WIRE START // %s.%s. Pick the second pin." % [str(body.get_meta("refdes", "?")), pin_label]
        return

    if body == wire_start and pin_id == wire_start_pin:
        if status_label != null:
            status_label.text = "WIRE: choose a different endpoint."
        return

    _connect_pin_endpoints(wire_start, wire_start_pin, body, pin_id)
    wire_start = null
    wire_start_pin = ""

func _connect_pin_endpoints(a: StaticBody3D, a_pin: String, b: StaticBody3D, b_pin: String) -> bool:
    if a == null or b == null or not is_instance_valid(a) or not is_instance_valid(b):
        return false
    if a_pin.is_empty() or b_pin.is_empty():
        return false
    for wire in user_wires:
        var wa := wire.get("a") as StaticBody3D
        var wb := wire.get("b") as StaticBody3D
        var wap := str(wire.get("a_pin", ""))
        var wbp := str(wire.get("b_pin", ""))
        var same_forward := wa == a and wb == b and wap == a_pin and wbp == b_pin
        var same_reverse := wa == b and wb == a and wap == b_pin and wbp == a_pin
        if same_forward or same_reverse:
            if status_label != null:
                status_label.text = "WIRE: that exact pin connection already exists."
            return false

    var start := _pin_root_position(a, a_pin)
    var finish := _pin_root_position(b, b_pin)
    var node := _segment(
        "UserPinWire_%d" % (user_wires.size() + 1),
        start,
        finish,
        WIRE_VISUAL_RADIUS,
        _material(NORMAL_WIRE_COLOR, 0.52, 0.28)
    )
    node.set_meta("user_wire", true)
    user_wires.append({
        "node": node,
        "a": a,
        "a_pin": a_pin,
        "b": b,
        "b_pin": b_pin
    })
    _refresh_schematic()
    _export_engineering_snapshot()
    if status_label != null:
        status_label.text = "WIRE // %s.%s ↔ %s.%s" % [
            str(a.get_meta("refdes", "?")), a_pin,
            str(b.get_meta("refdes", "?")), b_pin
        ]
    return true

func _connect_parts(a: StaticBody3D, b: StaticBody3D) -> void:
    # Compatibility path used by older history/tests. New player wiring uses
    # exact pins; legacy calls connect the first declared pin on each part.
    var a_pin := _first_pin_id(a)
    var b_pin := _first_pin_id(b)
    _connect_pin_endpoints(a, a_pin, b, b_pin)

func _first_pin_id(body: StaticBody3D) -> String:
    if body == null:
        return ""
    var pins: Array = PIN_LIBRARY.get(str(body.get_meta("component_id", "")), [])
    if pins.is_empty():
        return ""
    return str((pins[0] as Dictionary).get("id", ""))

func _pin_root_position(body: StaticBody3D, pin_id: String) -> Vector3:
    _ensure_pin_ports(body)
    var marker := body.get_node_or_null("PinPorts1127/Pin_%s" % pin_id) as MeshInstance3D
    if marker == null:
        return body.position + Vector3(0.0, WIRE_VISUAL_OFFSET_Y, 0.0)
    return to_local(marker.global_position)

func _refresh_wires_for_component(body: StaticBody3D) -> void:
    for wire in user_wires:
        var a := wire.get("a") as StaticBody3D
        var b := wire.get("b") as StaticBody3D
        if a != body and b != body:
            continue
        var old_node := wire.get("node") as Node
        if old_node != null and is_instance_valid(old_node):
            old_node.queue_free()
        if a == null or b == null or not is_instance_valid(a) or not is_instance_valid(b):
            continue
        var start := _pin_root_position(a, str(wire.get("a_pin", _first_pin_id(a))))
        var finish := _pin_root_position(b, str(wire.get("b_pin", _first_pin_id(b))))
        var new_node := _segment(
            "UserPinWire_%d" % (user_wires.find(wire) + 1),
            start,
            finish,
            WIRE_VISUAL_RADIUS,
            _material(NORMAL_WIRE_COLOR, 0.52, 0.28)
        )
        new_node.set_meta("user_wire", true)
        wire["node"] = new_node

func _remove_wires_for(body: StaticBody3D) -> void:
    var keep: Array[Dictionary] = []
    for wire in user_wires:
        if wire.get("a") == body or wire.get("b") == body:
            var node := wire.get("node") as Node
            if node != null and is_instance_valid(node):
                node.queue_free()
        else:
            keep.append(wire)
    user_wires = keep
    _refresh_schematic()

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

    var connections: Array = []
    for wire in user_wires:
        var a := wire.get("a") as StaticBody3D
        var b := wire.get("b") as StaticBody3D
        if a == null or b == null or not is_instance_valid(a) or not is_instance_valid(b):
            continue
        connections.append({
            "a_ref": str(a.get_meta("refdes", "?")),
            "a_pin": str(wire.get("a_pin", _first_pin_id(a))),
            "b_ref": str(b.get_meta("refdes", "?")),
            "b_pin": str(wire.get("b_pin", _first_pin_id(b)))
        })

    if schematic_canvas.has_method("set_circuit_graph"):
        schematic_canvas.call("set_circuit_graph", rows, connections)

func _endpoint(body: StaticBody3D, pin_id: String) -> String:
    return "%s.%s" % [str(body.get_meta("refdes", "")), pin_id]

func _build_pin_graph(include_internal_topology := false) -> RefCounted:
    var graph = PIN_GRAPH_1127.new()
    for body in placed_components:
        if body == null or not is_instance_valid(body):
            continue
        var component_id := str(body.get_meta("component_id", ""))
        var pins: Array = PIN_LIBRARY.get(component_id, [])
        for pin_variant in pins:
            var pin := pin_variant as Dictionary
            graph.add_pin(_endpoint(body, str(pin.get("id", ""))))
        if include_internal_topology and INTERNAL_TOPOLOGY_PAIRS.has(component_id):
            for pair_variant in INTERNAL_TOPOLOGY_PAIRS[component_id]:
                var pair := pair_variant as Array
                graph.connect_pins(
                    _endpoint(body, str(pair[0])),
                    _endpoint(body, str(pair[1]))
                )

    for wire in user_wires:
        var a := wire.get("a") as StaticBody3D
        var b := wire.get("b") as StaticBody3D
        if a == null or b == null or not is_instance_valid(a) or not is_instance_valid(b):
            continue
        graph.connect_pins(
            _endpoint(a, str(wire.get("a_pin", _first_pin_id(a)))),
            _endpoint(b, str(wire.get("b_pin", _first_pin_id(b))))
        )
    return graph

func _test_player_circuit() -> void:
    last_pin_precheck_passed = false
    _reset_player_test_visuals()
    var graph = _build_pin_graph(true)
    var powers: Array[StaticBody3D] = []
    var leds: Array[StaticBody3D] = []
    for body in placed_components:
        if body == null or not is_instance_valid(body):
            continue
        var component_id := str(body.get_meta("component_id", ""))
        if component_id == "power":
            powers.append(body)
        elif component_id == "led":
            leds.append(body)

    if powers.is_empty() or leds.is_empty():
        if build_status != null:
            build_status.text = "TEST PRECHECK: add POWER + LED"
        if status_label != null:
            status_label.text = "Pin precheck needs at least one DC power source and one LED."
        return

    for power in powers:
        for led in leds:
            var forward_path: PackedStringArray = graph.shortest_path(_endpoint(power, "POS"), _endpoint(led, "A"))
            var return_path: PackedStringArray = graph.shortest_path(_endpoint(led, "K"), _endpoint(power, "NEG"))
            if not forward_path.is_empty() and not return_path.is_empty():
                last_pin_precheck_passed = true
                if build_status != null:
                    build_status.text = "TEST PRECHECK: CLOSED PIN LOOP"
                if status_label != null:
                    status_label.text = "Pin topology found a + → LED anode and LED cathode → - loop. This is topology only; C++ validation/simulation still decides engineering truth."
                _export_engineering_snapshot()
                return

    if build_status != null:
        build_status.text = "TEST PRECHECK: OPEN / WRONG PIN"
    if status_label != null:
        status_label.text = "No complete pin-level LED loop found. Check +, -, LED A/K, and any series-part pins."

func _capture_board_snapshot() -> Dictionary:
    var components: Array[Dictionary] = []
    for body in placed_components:
        if body == null or not is_instance_valid(body):
            continue
        components.append({
            "component_id": str(body.get_meta("component_id", "")),
            "refdes": str(body.get_meta("refdes", "part")),
            "x": body.position.x,
            "y": body.position.y,
            "z": body.position.z,
            "rotation_y": body.rotation.y
        })

    var wires: Array[Dictionary] = []
    for wire in user_wires:
        var a := wire.get("a") as StaticBody3D
        var b := wire.get("b") as StaticBody3D
        if a == null or b == null or not is_instance_valid(a) or not is_instance_valid(b):
            continue
        wires.append({
            "a": str(a.get_meta("refdes", "?")),
            "a_pin": str(wire.get("a_pin", _first_pin_id(a))),
            "b": str(b.get_meta("refdes", "?")),
            "b_pin": str(wire.get("b_pin", _first_pin_id(b)))
        })
    return {"format":"formfactor-blueprint-v1", "components": components, "wires": wires}

func _restore_board_snapshot(snapshot: Dictionary) -> void:
    history_replaying = true
    hovered_component = null
    active_component = null
    dragging_component = null
    _clear_player_board()

    reference_counts.clear()
    for component in COMPONENTS:
        reference_counts[str(component.get("prefix", "X"))] = 0

    var bodies_by_refdes: Dictionary = {}
    var component_states: Array = snapshot.get("components", [])
    for state_variant in component_states:
        var state := state_variant as Dictionary
        var component_id := str(state.get("component_id", ""))
        var saved_refdes := str(state.get("refdes", "part"))
        selected_component_id = component_id
        var body := _place_component_at_world(Vector3(
            float(state.get("x", 0.0)),
            float(state.get("y", BOARD_Y)),
            float(state.get("z", 0.0))
        ))
        if body == null:
            continue
        body.rotation.y = float(state.get("rotation_y", 0.0))
        body.set_meta("refdes", saved_refdes)
        _ensure_pin_ports(body)
        _apply_engineering_link_metadata(body)
        bodies_by_refdes[saved_refdes] = body

        var component := _component_by_id(component_id)
        var prefix := str(component.get("prefix", "X"))
        if saved_refdes.begins_with(prefix):
            var number := saved_refdes.substr(prefix.length()).to_int()
            reference_counts[prefix] = maxi(int(reference_counts.get(prefix, 0)), number)

    var wire_states: Array = snapshot.get("wires", [])
    for wire_variant in wire_states:
        var wire_state := wire_variant as Dictionary
        var a := bodies_by_refdes.get(str(wire_state.get("a", ""))) as StaticBody3D
        var b := bodies_by_refdes.get(str(wire_state.get("b", ""))) as StaticBody3D
        if a == null or b == null:
            continue
        _connect_pin_endpoints(
            a,
            str(wire_state.get("a_pin", _first_pin_id(a))),
            b,
            str(wire_state.get("b_pin", _first_pin_id(b)))
        )

    selected_component_id = ""
    wire_mode = false
    wire_start = null
    wire_start_pin = ""
    active_component = null
    dragging_component = null
    _reset_player_test_visuals()
    _refresh_inventory()
    _refresh_schematic()
    _update_selected_label()

    history_signature = JSON.stringify(snapshot)
    observed_signature = history_signature
    history_settle_timer = 0.0
    history_replaying = false
    _update_history_controls()
    _update_context_target(null)
    _set_editor_tool(EditorTool.SELECT)
    _export_engineering_snapshot()

func _commit_history_snapshot() -> bool:
    var committed := super._commit_history_snapshot()
    if committed:
        _export_engineering_snapshot()
    return committed

func _save_blueprint() -> void:
    DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path("user://engineering_blueprints"))
    var file := FileAccess.open(BLUEPRINT_PATH, FileAccess.WRITE)
    if file == null:
        if status_label != null:
            status_label.text = "Blueprint save failed."
        return
    file.store_string(JSON.stringify(_capture_board_snapshot(), "  ", false))
    file.close()
    if status_label != null:
        status_label.text = "Engineering Blueprint saved. LOAD BP can restore this exact pin-wired board."

func _load_blueprint() -> void:
    if not FileAccess.file_exists(BLUEPRINT_PATH):
        if status_label != null:
            status_label.text = "No saved Engineering Blueprint yet."
        return
    var file := FileAccess.open(BLUEPRINT_PATH, FileAccess.READ)
    if file == null:
        return
    var parsed = JSON.parse_string(file.get_as_text())
    file.close()
    if parsed is not Dictionary:
        if status_label != null:
            status_label.text = "Blueprint file is invalid."
        return
    _restore_board_snapshot(parsed as Dictionary)
    if status_label != null:
        status_label.text = "Engineering Blueprint restored with exact pin endpoints."

func _export_engineering_snapshot() -> bool:
    if history_replaying:
        return false
    var graph = _build_pin_graph(false)
    var snapshot := ENGINEERING_BRIDGE_1127.build_snapshot(
        placed_components,
        user_wires,
        PIN_LIBRARY,
        PACKAGE_LINKS,
        graph.named_nets()
    )
    return ENGINEERING_BRIDGE_1127.write_snapshot(snapshot, ENGINEERING_SNAPSHOT_PATH)

func _update_context_target(target: StaticBody3D) -> void:
    super._update_context_target(target)
    var use_target := target
    if use_target == null and active_component != null and is_instance_valid(active_component):
        use_target = active_component
    if use_target != null and is_instance_valid(use_target):
        _show_pin_details(use_target)

func debug_pin_catalog_valid() -> bool:
    return pin_catalog_valid

func debug_pin_count_for(component_id: String) -> int:
    return (PIN_LIBRARY.get(component_id, []) as Array).size()

func debug_connect_pin_refs(a_ref: String, a_pin: String, b_ref: String, b_pin: String) -> bool:
    var a := debug_find_component(a_ref)
    var b := debug_find_component(b_ref)
    if a == null or b == null:
        return false
    return _connect_pin_endpoints(a, a_pin, b, b_pin)

func debug_wire_pin_pair(index: int) -> PackedStringArray:
    if index < 0 or index >= user_wires.size():
        return PackedStringArray()
    var wire := user_wires[index]
    return PackedStringArray([str(wire.get("a_pin", "")), str(wire.get("b_pin", ""))])

func debug_named_net_count() -> int:
    return (_build_pin_graph(false).named_nets() as Array).size()

func debug_snapshot_exported() -> bool:
    return FileAccess.file_exists(ENGINEERING_SNAPSHOT_PATH)

func debug_last_pin_precheck_passed() -> bool:
    return last_pin_precheck_passed

func debug_blueprint_roundtrip_ready() -> bool:
    _save_blueprint()
    return FileAccess.file_exists(BLUEPRINT_PATH)

func debug_truth_gate_count() -> int:
    return 7
