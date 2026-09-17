extends "res://scripts/material_lab.gd"

const BOARD_Y := 0.24
const BOARD_X_LIMIT := 4.05
const BOARD_Z_LIMIT := 2.42
const GRID_STEP := 0.50
const PAN_STEP := 0.55
const DRAG_THRESHOLD := 7.0

const COMPONENTS := [
    {"id":"resistor","name":"Resistor","prefix":"R","category":"Passive","symbol":"--/\\/\\--","stock":12},
    {"id":"potentiometer","name":"Potentiometer","prefix":"RV","category":"Passive","symbol":"--/\\/\\<-","stock":6},
    {"id":"ceramic_cap","name":"Ceramic capacitor","prefix":"C","category":"Passive","symbol":"--| |--","stock":10},
    {"id":"electrolytic_cap","name":"Electrolytic capacitor","prefix":"C","category":"Passive","symbol":"--|(|-- +","stock":8},
    {"id":"inductor","name":"Inductor","prefix":"L","category":"Passive","symbol":"--oooo--","stock":8},
    {"id":"diode","name":"Standard diode","prefix":"D","category":"Semiconductor","symbol":"--|>|--","stock":10},
    {"id":"zener","name":"Zener diode","prefix":"D","category":"Semiconductor","symbol":"--|>|-|","stock":6},
    {"id":"led","name":"LED","prefix":"D","category":"Semiconductor","symbol":"--|>|-- >>","stock":8},
    {"id":"npn","name":"NPN transistor","prefix":"Q","category":"Semiconductor","symbol":"NPN >","stock":6},
    {"id":"pnp","name":"PNP transistor","prefix":"Q","category":"Semiconductor","symbol":"PNP <","stock":6},
    {"id":"nmos","name":"N-channel MOSFET","prefix":"Q","category":"Semiconductor","symbol":"G-| |-D N","stock":6},
    {"id":"pmos","name":"P-channel MOSFET","prefix":"Q","category":"Semiconductor","symbol":"G-| |-D P","stock":6},
    {"id":"logic","name":"Logic gate / inverter","prefix":"U","category":"Logic","symbol":"-[>o]-","stock":6},
    {"id":"opamp","name":"Operational amplifier","prefix":"U","category":"Logic","symbol":"--|>-- +/-","stock":6},
    {"id":"comparator","name":"Comparator","prefix":"U","category":"Logic","symbol":"--|>-- CMP","stock":6},
    {"id":"regulator","name":"Voltage regulator","prefix":"U","category":"Power","symbol":"IN-[REG]-OUT","stock":6},
    {"id":"fuse","name":"Fuse","prefix":"F","category":"Power","symbol":"--[ F ]--","stock":8},
    {"id":"switch","name":"Switch","prefix":"SW","category":"Power","symbol":"--o/ o--","stock":8},
    {"id":"relay","name":"Relay","prefix":"K","category":"Power","symbol":"COIL -> SW","stock":5},
    {"id":"connector","name":"Connector / header","prefix":"J","category":"Interface","symbol":"o o o o","stock":8},
    {"id":"test_point","name":"Test point","prefix":"TP","category":"Interface","symbol":"--o TP","stock":12},
    {"id":"sensor","name":"Sensor","prefix":"S","category":"Sensor","symbol":"[SENSE]","stock":6},
    {"id":"buzzer","name":"Buzzer / output","prefix":"BZ","category":"Output","symbol":"( )))","stock":5},
    {"id":"power","name":"DC power source","prefix":"PWR","category":"Power","symbol":"+ | | -","stock":4},
    {"id":"ground","name":"Ground / reference","prefix":"GND","category":"Power","symbol":"--|_ GND","stock":20}
]

var selected_component_id := ""
var inventory_stock: Dictionary = {}
var reference_counts: Dictionary = {}
var placed_components: Array[StaticBody3D] = []
var user_wires: Array[Dictionary] = []
var wire_mode := false
var wire_start: StaticBody3D = null
var pointer_down := false
var pointer_dragged := false
var pointer_start := Vector2.ZERO
var last_pointer := Vector2.ZERO
var inventory_list: VBoxContainer
var search_box: LineEdit
var category_filter: OptionButton
var schematic_label: Label
var build_status: Label
var selected_label: Label

func _ready() -> void:
    super._ready()
    for component in COMPONENTS:
        inventory_stock[component.id] = int(component.stock)
        if not reference_counts.has(component.prefix):
            reference_counts[component.prefix] = 0
    _retitle_existing_lab()
    _build_gameplay_hud()
    _refresh_inventory()
    _refresh_schematic()
    status_label.text = "Build mode ready. Pick a part from inventory, then click the PCB to place it."

func _process(delta_seconds: float) -> void:
    super._process(delta_seconds)

func _unhandled_input(event: InputEvent) -> void:
    if event is InputEventMouseButton:
        var mb := event as InputEventMouseButton
        if mb.button_index == MOUSE_BUTTON_WHEEL_UP and mb.pressed:
            camera.position.z = maxf(4.2, camera.position.z - 0.45)
            get_viewport().set_input_as_handled()
            return
        if mb.button_index == MOUSE_BUTTON_WHEEL_DOWN and mb.pressed:
            camera.position.z = minf(12.5, camera.position.z + 0.45)
            get_viewport().set_input_as_handled()
            return
        if mb.button_index == MOUSE_BUTTON_LEFT:
            if mb.pressed:
                pointer_down = true
                pointer_dragged = false
                pointer_start = mb.position
                last_pointer = mb.position
            else:
                var was_dragged := pointer_dragged
                pointer_down = false
                pointer_dragged = false
                if not was_dragged:
                    _handle_world_click(mb.position)
            get_viewport().set_input_as_handled()
            return
        if mb.button_index == MOUSE_BUTTON_RIGHT and mb.pressed:
            _remove_at_screen(mb.position)
            get_viewport().set_input_as_handled()
            return
    elif event is InputEventMouseMotion and pointer_down:
        var mm := event as InputEventMouseMotion
        if not pointer_dragged and pointer_start.distance_to(mm.position) >= DRAG_THRESHOLD:
            pointer_dragged = true
        if pointer_dragged:
            var mouse_delta := mm.position - last_pointer
            camera_pivot.rotation.y -= mouse_delta.x * 0.007
            camera_pivot.rotation.x = clampf(camera_pivot.rotation.x - mouse_delta.y * 0.005, -0.55, 0.78)
        last_pointer = mm.position
        get_viewport().set_input_as_handled()
        return
    elif event is InputEventKey:
        var key := event as InputEventKey
        if not key.pressed or key.echo:
            return
        match key.keycode:
            KEY_RIGHT, KEY_D:
                _pan_view(Vector2(1.0, 0.0))
            KEY_LEFT, KEY_A:
                _pan_view(Vector2(-1.0, 0.0))
            KEY_UP:
                _pan_view(Vector2(0.0, -1.0))
            KEY_DOWN:
                _pan_view(Vector2(0.0, 1.0))
            KEY_W:
                _toggle_wire_mode()
            KEY_R:
                _reset_view()
            KEY_C:
                _clear_player_board()
            KEY_ESCAPE:
                selected_component_id = ""
                wire_mode = false
                wire_start = null
                _update_selected_label()
        get_viewport().set_input_as_handled()

func _retitle_existing_lab() -> void:
    var hud := get_node_or_null("HUD")
    if hud == null:
        return
    for child in hud.find_children("*", "Label", true, false):
        var label := child as Label
        if label != null and label.text.begins_with("FORMFACTOR 1.06"):
            label.text = "FORMFACTOR 1.07 // INTERACTIVE 3D BUILD LAB"
    _label3d("1.07 // BUILD MODE", Vector3(2.25, 0.19, -2.08), 0.008, NEON)

func _build_gameplay_hud() -> void:
    var hud := get_node_or_null("HUD") as CanvasLayer
    if hud == null:
        return

    var inventory_panel := ColorRect.new()
    inventory_panel.name = "InventoryPanel"
    inventory_panel.position = Vector2(18, 226)
    inventory_panel.size = Vector2(370, 580)
    inventory_panel.color = Color(0.012, 0.021, 0.026, 0.95)
    hud.add_child(inventory_panel)

    var inv_title := Label.new()
    inv_title.position = Vector2(14, 10)
    inv_title.text = "COMPONENT INVENTORY // 25 FAMILIES"
    inv_title.add_theme_color_override("font_color", NEON)
    inv_title.add_theme_font_size_override("font_size", 16)
    inventory_panel.add_child(inv_title)

    search_box = LineEdit.new()
    search_box.position = Vector2(14, 40)
    search_box.size = Vector2(220, 34)
    search_box.placeholder_text = "Search parts or symbols"
    search_box.text_changed.connect(func(_text: String) -> void: _refresh_inventory())
    inventory_panel.add_child(search_box)

    category_filter = OptionButton.new()
    category_filter.position = Vector2(240, 40)
    category_filter.size = Vector2(116, 34)
    for category in ["All", "Passive", "Semiconductor", "Logic", "Power", "Interface", "Sensor", "Output"]:
        category_filter.add_item(category)
    category_filter.item_selected.connect(func(_index: int) -> void: _refresh_inventory())
    inventory_panel.add_child(category_filter)

    var scroll := ScrollContainer.new()
    scroll.position = Vector2(14, 82)
    scroll.size = Vector2(342, 438)
    inventory_panel.add_child(scroll)

    inventory_list = VBoxContainer.new()
    inventory_list.custom_minimum_size = Vector2(322, 0)
    inventory_list.add_theme_constant_override("separation", 4)
    scroll.add_child(inventory_list)

    selected_label = Label.new()
    selected_label.position = Vector2(14, 530)
    selected_label.size = Vector2(342, 42)
    selected_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    selected_label.add_theme_color_override("font_color", WHITE)
    selected_label.add_theme_font_size_override("font_size", 13)
    inventory_panel.add_child(selected_label)

    var schematic_panel := ColorRect.new()
    schematic_panel.name = "SchematicMirrorPanel"
    schematic_panel.position = Vector2(1090, 112)
    schematic_panel.size = Vector2(368, 410)
    schematic_panel.color = Color(0.012, 0.021, 0.026, 0.94)
    hud.add_child(schematic_panel)

    var schematic_title := Label.new()
    schematic_title.position = Vector2(14, 10)
    schematic_title.text = "LIVE SCHEMATIC MIRROR"
    schematic_title.add_theme_color_override("font_color", NEON)
    schematic_title.add_theme_font_size_override("font_size", 16)
    schematic_panel.add_child(schematic_title)

    schematic_label = Label.new()
    schematic_label.position = Vector2(14, 42)
    schematic_label.size = Vector2(340, 350)
    schematic_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    schematic_label.add_theme_color_override("font_color", Color(0.78, 0.86, 0.88, 1.0))
    schematic_label.add_theme_font_size_override("font_size", 13)
    schematic_panel.add_child(schematic_label)

    var tool_panel := ColorRect.new()
    tool_panel.name = "BuildToolsPanel"
    tool_panel.position = Vector2(1090, 536)
    tool_panel.size = Vector2(368, 260)
    tool_panel.color = Color(0.012, 0.021, 0.026, 0.94)
    hud.add_child(tool_panel)

    var tool_title := Label.new()
    tool_title.position = Vector2(14, 10)
    tool_title.text = "BUILD + VIEW CONTROLS"
    tool_title.add_theme_color_override("font_color", NEON)
    tool_title.add_theme_font_size_override("font_size", 16)
    tool_panel.add_child(tool_title)

    _hud_button(tool_panel, "←", Vector2(14, 48), Vector2(52, 42), func() -> void: _pan_view(Vector2(-1, 0)))
    _hud_button(tool_panel, "↑", Vector2(72, 48), Vector2(52, 42), func() -> void: _pan_view(Vector2(0, -1)))
    _hud_button(tool_panel, "↓", Vector2(130, 48), Vector2(52, 42), func() -> void: _pan_view(Vector2(0, 1)))
    _hud_button(tool_panel, "→", Vector2(188, 48), Vector2(52, 42), func() -> void: _pan_view(Vector2(1, 0)))
    _hud_button(tool_panel, "RESET VIEW", Vector2(246, 48), Vector2(108, 42), _reset_view)

    _hud_button(tool_panel, "WIRE W", Vector2(14, 100), Vector2(100, 40), _toggle_wire_mode)
    _hud_button(tool_panel, "TEST", Vector2(122, 100), Vector2(72, 40), _test_player_circuit)
    _hud_button(tool_panel, "CLEAR C", Vector2(202, 100), Vector2(96, 40), _clear_player_board)

    var hint := Label.new()
    hint.position = Vector2(14, 150)
    hint.size = Vector2(340, 54)
    hint.text = "ARROWS/A/D = pan • CLICK + DRAG = rotate\nWHEEL = zoom • RIGHT CLICK = remove part"
    hint.add_theme_color_override("font_color", Color(0.68, 0.78, 0.80, 1.0))
    hint.add_theme_font_size_override("font_size", 12)
    tool_panel.add_child(hint)

    build_status = Label.new()
    build_status.position = Vector2(14, 207)
    build_status.size = Vector2(340, 42)
    build_status.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    build_status.add_theme_color_override("font_color", WHITE)
    build_status.add_theme_font_size_override("font_size", 12)
    tool_panel.add_child(build_status)
    _update_selected_label()

func _hud_button(parent: Control, text_value: String, pos: Vector2, size_value: Vector2, callback: Callable) -> Button:
    var button := Button.new()
    button.text = text_value
    button.position = pos
    button.size = size_value
    button.pressed.connect(callback)
    parent.add_child(button)
    return button

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
        if category != "All" and component.category != category:
            continue
        var haystack := (str(component.name) + " " + str(component.prefix) + " " + str(component.symbol) + " " + str(component.category)).to_lower()
        if not query.is_empty() and haystack.find(query) == -1:
            continue
        var count := int(inventory_stock.get(component.id, 0))
        var button := Button.new()
        button.name = "Inventory_%s" % component.id
        button.text = "%s  %-24s x%-2d   %s" % [component.prefix, component.name, count, component.symbol]
        button.alignment = HORIZONTAL_ALIGNMENT_LEFT
        button.custom_minimum_size = Vector2(322, 34)
        button.disabled = count <= 0
        var id := str(component.id)
        button.pressed.connect(func() -> void: select_component(id))
        inventory_list.add_child(button)

func select_component(component_id: String) -> void:
    if int(inventory_stock.get(component_id, 0)) <= 0:
        return
    selected_component_id = component_id
    wire_mode = false
    wire_start = null
    _update_selected_label()
    var component := _component_by_id(component_id)
    if not component.is_empty():
        status_label.text = "Selected %s. Click the PCB to place it; drag instead to rotate the workbench." % component.name

func _update_selected_label() -> void:
    if selected_label == null:
        return
    if wire_mode:
        selected_label.text = "ACTIVE TOOL: WIRE — click two placed parts"
    elif selected_component_id.is_empty():
        selected_label.text = "ACTIVE TOOL: none — choose a component"
    else:
        var component := _component_by_id(selected_component_id)
        if component.is_empty():
            selected_label.text = "ACTIVE TOOL: none"
        else:
            selected_label.text = "ACTIVE PART: %s %s  •  %s" % [component.prefix, component.name, component.symbol]

func _component_by_id(component_id: String) -> Dictionary:
    for component in COMPONENTS:
        if component.id == component_id:
            return component
    return {}

func _handle_world_click(screen_pos: Vector2) -> void:
    if wire_mode:
        _wire_click(screen_pos)
        return
    if not selected_component_id.is_empty():
        var board_pos := _screen_to_board(screen_pos)
        if board_pos != Vector3.INF:
            _place_component_at_world(board_pos)
            return
    _pick_part(screen_pos)

func _screen_to_board(screen_pos: Vector2) -> Vector3:
    var origin := camera.project_ray_origin(screen_pos)
    var direction := camera.project_ray_normal(screen_pos)
    if absf(direction.y) < 0.0001:
        return Vector3.INF
    var distance := (BOARD_Y - origin.y) / direction.y
    if distance <= 0.0:
        return Vector3.INF
    var hit := origin + direction * distance
    if absf(hit.x) > BOARD_X_LIMIT or absf(hit.z) > BOARD_Z_LIMIT:
        status_label.text = "Place parts inside the green PCB area."
        return Vector3.INF
    hit.x = snappedf(hit.x, GRID_STEP)
    hit.z = snappedf(hit.z, GRID_STEP)
    hit.y = BOARD_Y
    return hit

func _place_component_at_world(world_pos: Vector3) -> StaticBody3D:
    if selected_component_id.is_empty():
        return null
    if int(inventory_stock.get(selected_component_id, 0)) <= 0:
        status_label.text = "No stock remaining for that part."
        return null
    var component := _component_by_id(selected_component_id)
    if component.is_empty():
        return null
    var prefix := str(component.prefix)
    reference_counts[prefix] = int(reference_counts.get(prefix, 0)) + 1
    var refdes := "%s%d" % [prefix, int(reference_counts[prefix])]

    var body := StaticBody3D.new()
    body.name = "Placed_%s_%s" % [component.id, refdes]
    body.position = world_pos
    body.set_meta("placed_component", true)
    body.set_meta("component_id", component.id)
    body.set_meta("refdes", refdes)
    body.set_meta("part_id", component.id)
    add_child(body)

    var collision := CollisionShape3D.new()
    var shape := BoxShape3D.new()
    shape.size = Vector3(0.72, 0.55, 0.62)
    collision.position = Vector3(0, 0.20, 0)
    collision.shape = shape
    body.add_child(collision)

    _build_component_visual(body, component, refdes)
    placed_components.append(body)
    inventory_stock[component.id] = int(inventory_stock[component.id]) - 1
    selected_component_id = ""
    _refresh_inventory()
    _refresh_schematic()
    _update_selected_label()
    status_label.text = "%s placed. Choose another part, wire parts together, or TEST the board." % refdes
    return body

func _build_component_visual(body: StaticBody3D, component: Dictionary, refdes: String) -> void:
    var id := str(component.id)
    var body_mat := _material(_component_color(component.category), 0.18, 0.42)
    var metal := _material(Color(0.72, 0.74, 0.76, 1), 0.92, 0.16)
    if id in ["electrolytic_cap", "inductor", "fuse", "buzzer", "test_point"]:
        _cylinder_child(body, "Body3D", 0.24, 0.42, Vector3(0, 0.24, 0), body_mat)
    elif id == "led":
        var sphere := MeshInstance3D.new()
        var mesh := SphereMesh.new()
        mesh.radius = 0.24
        mesh.height = 0.45
        sphere.mesh = mesh
        sphere.position = Vector3(0, 0.32, 0)
        sphere.material_override = _material(Color(0.86, 0.10, 0.08, 1), 0.02, 0.18)
        body.add_child(sphere)
    elif id in ["npn", "pnp", "nmos", "pmos"]:
        _box_child(body, "Package3D", Vector3(0.52, 0.42, 0.28), Vector3(0, 0.27, 0), body_mat)
        for x in [-0.20, 0.0, 0.20]:
            _box_child(body, "Lead3D", Vector3(0.04, 0.25, 0.04), Vector3(x, 0.05, 0), metal)
    elif id in ["logic", "opamp", "comparator", "regulator"]:
        _box_child(body, "ICPackage3D", Vector3(0.72, 0.28, 0.52), Vector3(0, 0.24, 0), body_mat)
        for x in [-0.28, -0.09, 0.09, 0.28]:
            _box_child(body, "ICPin3D", Vector3(0.035, 0.08, 0.16), Vector3(x, 0.11, 0.31), metal)
            _box_child(body, "ICPin3D", Vector3(0.035, 0.08, 0.16), Vector3(x, 0.11, -0.31), metal)
    elif id == "connector":
        _box_child(body, "Header3D", Vector3(0.72, 0.22, 0.38), Vector3(0, 0.20, 0), body_mat)
        for x in [-0.27, -0.09, 0.09, 0.27]:
            _cylinder_child(body, "Pin3D", 0.035, 0.28, Vector3(x, 0.30, 0), metal)
    elif id == "ground":
        _cylinder_child(body, "GroundPad3D", 0.28, 0.06, Vector3(0, 0.05, 0), metal)
        _box_child(body, "GroundStem3D", Vector3(0.05, 0.30, 0.05), Vector3(0, 0.18, 0), metal)
    elif id == "resistor":
        var resistor := _cylinder_child(body, "ResistorBody3D", 0.15, 0.52, Vector3(0, 0.22, 0), body_mat)
        resistor.rotation_degrees.z = 90.0
        _box_child(body, "LeadL3D", Vector3(0.28, 0.04, 0.04), Vector3(-0.38, 0.22, 0), metal)
        _box_child(body, "LeadR3D", Vector3(0.28, 0.04, 0.04), Vector3(0.38, 0.22, 0), metal)
    else:
        _box_child(body, "Body3D", Vector3(0.62, 0.34, 0.46), Vector3(0, 0.23, 0), body_mat)
        _box_child(body, "LeadL3D", Vector3(0.24, 0.04, 0.04), Vector3(-0.42, 0.17, 0), metal)
        _box_child(body, "LeadR3D", Vector3(0.24, 0.04, 0.04), Vector3(0.42, 0.17, 0), metal)

    var label := _label3d_child(body, refdes, Vector3(-0.24, 0.56, 0), 0.0065, WHITE)
    label.rotation_degrees = Vector3(-90, 0, 0)

func _component_color(category: String) -> Color:
    match category:
        "Passive": return Color(0.74, 0.55, 0.20, 1)
        "Semiconductor": return Color(0.12, 0.28, 0.38, 1)
        "Logic": return Color(0.10, 0.12, 0.14, 1)
        "Power": return Color(0.42, 0.12, 0.10, 1)
        "Interface": return Color(0.16, 0.26, 0.30, 1)
        "Sensor": return Color(0.22, 0.34, 0.20, 1)
        "Output": return Color(0.42, 0.20, 0.36, 1)
        _: return Color(0.24, 0.26, 0.28, 1)

func _ray_collider(screen_pos: Vector2) -> Object:
    var origin := camera.project_ray_origin(screen_pos)
    var ray_end := origin + camera.project_ray_normal(screen_pos) * 100.0
    var query := PhysicsRayQueryParameters3D.create(origin, ray_end)
    query.collide_with_bodies = true
    var hit := get_world_3d().direct_space_state.intersect_ray(query)
    if hit.is_empty():
        return null
    return hit.get("collider") as Object

func _wire_click(screen_pos: Vector2) -> void:
    var collider := _ray_collider(screen_pos)
    if collider == null or not bool(collider.get_meta("placed_component", false)):
        status_label.text = "Wire tool: click a component you placed on the PCB."
        return
    var body := collider as StaticBody3D
    if body == null:
        return
    if wire_start == null:
        wire_start = body
        status_label.text = "Wire start: %s. Click a second placed part." % str(body.get_meta("refdes", "part"))
        return
    if body == wire_start:
        status_label.text = "Choose a different component for the wire endpoint."
        return
    _connect_parts(wire_start, body)
    wire_start = null

func _connect_parts(a: StaticBody3D, b: StaticBody3D) -> void:
    var from_point := a.position + Vector3(0, 0.70, 0)
    var to_point := b.position + Vector3(0, 0.70, 0)
    var node := _segment("UserWire_%d" % (user_wires.size() + 1), from_point, to_point, 0.035,
        _material(Color(0.95, 0.34, 0.06, 1), 0.52, 0.28))
    node.set_meta("user_wire", true)
    user_wires.append({"node":node, "a":a, "b":b})
    _refresh_schematic()
    status_label.text = "Wire connected: %s ↔ %s" % [str(a.get_meta("refdes", "?")), str(b.get_meta("refdes", "?"))]

func _toggle_wire_mode() -> void:
    wire_mode = not wire_mode
    selected_component_id = ""
    wire_start = null
    _update_selected_label()
    if build_status != null:
        build_status.text = "WIRE TOOL ON" if wire_mode else "WIRE TOOL OFF"

func _remove_at_screen(screen_pos: Vector2) -> void:
    var collider := _ray_collider(screen_pos)
    if collider == null or not bool(collider.get_meta("placed_component", false)):
        return
    var body := collider as StaticBody3D
    if body == null:
        return
    var component_id := str(body.get_meta("component_id", ""))
    inventory_stock[component_id] = int(inventory_stock.get(component_id, 0)) + 1
    _remove_wires_for(body)
    placed_components.erase(body)
    body.queue_free()
    _refresh_inventory()
    _refresh_schematic()
    status_label.text = "Removed component and returned it to inventory."

func _remove_wires_for(body: StaticBody3D) -> void:
    var keep: Array[Dictionary] = []
    for wire in user_wires:
        if wire.a == body or wire.b == body:
            var node := wire.node as Node
            if node != null:
                node.queue_free()
        else:
            keep.append(wire)
    user_wires = keep

func _clear_player_board() -> void:
    for wire in user_wires:
        var node := wire.node as Node
        if node != null:
            node.queue_free()
    user_wires.clear()
    for body in placed_components:
        var component_id := str(body.get_meta("component_id", ""))
        inventory_stock[component_id] = int(inventory_stock.get(component_id, 0)) + 1
        body.queue_free()
    placed_components.clear()
    wire_mode = false
    wire_start = null
    selected_component_id = ""
    _reset_tutorial()
    _refresh_inventory()
    _refresh_schematic()
    _update_selected_label()
    status_label.text = "Player board cleared. Inventory restored."

func _test_player_circuit() -> void:
    var has_power := false
    var has_led := false
    for body in placed_components:
        var id := str(body.get_meta("component_id", ""))
        has_power = has_power or id == "power"
        has_led = has_led or id == "led"
    if not has_power or not has_led:
        build_status.text = "TEST: add a DC power source and LED first."
        status_label.text = "Test failed safely: the player-built circuit needs both a power source and an LED."
        return
    if user_wires.is_empty():
        build_status.text = "TEST: add at least one wire."
        status_label.text = "Test failed safely: connect placed parts with the WIRE tool."
        return
    _complete_circuit()
    build_status.text = "TEST: circuit activity visible."
    status_label.text = "Basic visual test passed. Electrical truth/ratings still come from the validated engineering core."

func _refresh_schematic() -> void:
    if schematic_label == null:
        return
    if placed_components.is_empty():
        schematic_label.text = "Place a physical part and its schematic symbol will appear here.\n\nWIRE COUNT: 0\nBOARD: empty"
        return
    var lines: Array[String] = []
    var shown := 0
    for body in placed_components:
        if shown >= 14:
            lines.append("… +%d more parts" % (placed_components.size() - shown))
            break
        var id := str(body.get_meta("component_id", ""))
        var refdes := str(body.get_meta("refdes", "?"))
        var component := _component_by_id(id)
        lines.append("%-5s  %-14s  %s" % [refdes, str(component.symbol), str(component.name)])
        shown += 1
    lines.append("")
    lines.append("WIRES: %d" % user_wires.size())
    lines.append("PARTS: %d" % placed_components.size())
    schematic_label.text = "\n".join(lines)

func _pan_view(direction: Vector2) -> void:
    camera_pivot.position.x += direction.x * PAN_STEP
    camera_pivot.position.z += direction.y * PAN_STEP
    if build_status != null:
        build_status.text = "VIEW PAN: %.1f, %.1f" % [camera_pivot.position.x, camera_pivot.position.z]

func _reset_view() -> void:
    camera_pivot.position = Vector3.ZERO
    camera_pivot.rotation = Vector3(-0.18, -0.18, 0.0)
    camera.position = Vector3(0.0, 4.8, 8.4)
    if build_status != null:
        build_status.text = "VIEW RESET"

func debug_component_count() -> int:
    return COMPONENTS.size()

func debug_placed_count() -> int:
    return placed_components.size()

func debug_wire_count() -> int:
    return user_wires.size()

func debug_pan_position() -> Vector3:
    return camera_pivot.position
