extends Node3D

const GRID_STEP := 0.60
const BOARD_HALF_X := 5.40
const BOARD_HALF_Z := 3.00
const PART_Y := 0.28
const HISTORY_LIMIT := 64

var selected_kind := "LED"
var parts: Array[Node3D] = []
var selected_part: Node3D = null
var undo_stack: Array[Dictionary] = []
var camera: Camera3D
var status_label: Label
var bridge := EngineeringCoreBridge.new()

func _ready() -> void:
    _build_environment()
    _build_board()
    _build_ui()
    _spawn_part("POWER", Vector3(-2.4, PART_Y, 0.0), false)
    _set_status("GODOT OPEN PROTOTYPE — CLICK TO PLACE. E PICKS SAME. CTRL+D DUPLICATES. SHIFT+ARROWS NUDGE.")

func _build_environment() -> void:
    camera = Camera3D.new()
    camera.name = "PrototypeCamera"
    camera.position = Vector3(7.8, 7.2, 8.6)
    camera.look_at(Vector3.ZERO, Vector3.UP)
    add_child(camera)

    var light := DirectionalLight3D.new()
    light.rotation_degrees = Vector3(-55.0, -35.0, 0.0)
    light.light_energy = 1.8
    add_child(light)

    var fill := OmniLight3D.new()
    fill.position = Vector3(-3.0, 4.0, 2.0)
    fill.omni_range = 16.0
    fill.light_energy = 4.0
    fill.light_color = Color(0.22, 1.0, 0.22)
    add_child(fill)

func _build_board() -> void:
    var board := MeshInstance3D.new()
    board.name = "PrototypeBoard"
    var mesh := BoxMesh.new()
    mesh.size = Vector3(BOARD_HALF_X * 2.0 + 0.8, 0.18, BOARD_HALF_Z * 2.0 + 0.8)
    board.mesh = mesh
    board.position.y = 0.0
    var material := StandardMaterial3D.new()
    material.albedo_color = Color(0.035, 0.22, 0.12)
    material.metallic = 0.12
    material.roughness = 0.72
    board.material_override = material
    add_child(board)

func _build_ui() -> void:
    var layer := CanvasLayer.new()
    add_child(layer)

    status_label = Label.new()
    status_label.position = Vector2(22.0, 18.0)
    status_label.size = Vector2(1220.0, 90.0)
    status_label.add_theme_color_override("font_color", Color(0.28, 1.0, 0.12))
    status_label.add_theme_font_size_override("font_size", 18)
    status_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    layer.add_child(status_label)

    var help := Label.new()
    help.position = Vector2(22.0, 635.0)
    help.size = Vector2(1220.0, 70.0)
    help.text = "1 LED  2 RESISTOR  3 CAPACITOR  4 CHIP  5 POWER  |  E Pick Same  Ctrl+D Duplicate  Shift+Arrows Nudge  Ctrl+Z Undo  V Validate"
    help.add_theme_color_override("font_color", Color(0.82, 0.88, 0.90))
    help.add_theme_font_size_override("font_size", 15)
    layer.add_child(help)

func _unhandled_input(event: InputEvent) -> void:
    if event is InputEventMouseButton:
        if event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
            _handle_left_click(event.position)
        elif event.pressed and event.button_index == MOUSE_BUTTON_WHEEL_UP:
            camera.position *= 0.92
        elif event.pressed and event.button_index == MOUSE_BUTTON_WHEEL_DOWN:
            camera.position *= 1.08
        return

    if not (event is InputEventKey) or not event.pressed or event.echo:
        return

    var command := event.ctrl_pressed or event.meta_pressed
    if command and event.keycode == KEY_Z:
        _undo()
    elif command and event.keycode == KEY_D:
        _duplicate_selected()
    elif event.keycode == KEY_E:
        _pick_same_part()
    elif event.shift_pressed and event.keycode in [KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN]:
        _nudge_selected(event.keycode)
    elif event.keycode == KEY_V:
        _validate_with_core()
    elif event.keycode == KEY_1:
        selected_kind = "LED"
        _set_status("SELECTED LED — CLICK AN OPEN GRID SPOT.")
    elif event.keycode == KEY_2:
        selected_kind = "RESISTOR"
        _set_status("SELECTED RESISTOR — CLICK AN OPEN GRID SPOT.")
    elif event.keycode == KEY_3:
        selected_kind = "CAPACITOR"
        _set_status("SELECTED CAPACITOR — CLICK AN OPEN GRID SPOT.")
    elif event.keycode == KEY_4:
        selected_kind = "CHIP"
        _set_status("SELECTED CHIP — CLICK AN OPEN GRID SPOT.")
    elif event.keycode == KEY_5:
        selected_kind = "POWER"
        _set_status("SELECTED POWER — CLICK AN OPEN GRID SPOT.")

func _handle_left_click(screen_position: Vector2) -> void:
    var hit_part := _part_near_screen_position(screen_position)
    if hit_part != null:
        _select_part(hit_part)
        return

    var world := _board_point_from_screen(screen_position)
    if world == null:
        return

    var snapped := _snap_to_grid(world)
    if not _inside_board(snapped):
        _set_status("BLOCKED — OUTSIDE THE PROTOTYPE BOARD.")
        return
    if _position_occupied(snapped):
        _set_status("BLOCKED — ANOTHER COMPONENT ALREADY USES THAT GRID SPOT.")
        return

    _record_history()
    var part := _spawn_part(selected_kind, snapped, true)
    _select_part(part)
    _set_status("PLACED %s. CTRL+Z UNDOS IT." % selected_kind)

func _board_point_from_screen(screen_position: Vector2):
    var ray_origin := camera.project_ray_origin(screen_position)
    var ray_direction := camera.project_ray_normal(screen_position)
    var board_plane := Plane(Vector3.UP, PART_Y)
    return board_plane.intersects_ray(ray_origin, ray_direction)

func _snap_to_grid(world: Vector3) -> Vector3:
    return Vector3(
        round(world.x / GRID_STEP) * GRID_STEP,
        PART_Y,
        round(world.z / GRID_STEP) * GRID_STEP
    )

func _inside_board(position: Vector3) -> bool:
    return abs(position.x) <= BOARD_HALF_X and abs(position.z) <= BOARD_HALF_Z

func _position_occupied(position: Vector3, ignored: Node3D = null) -> bool:
    for part in parts:
        if part == ignored:
            continue
        if Vector2(part.position.x, part.position.z).distance_to(Vector2(position.x, position.z)) < GRID_STEP * 0.72:
            return true
    return false

func _part_near_screen_position(screen_position: Vector2) -> Node3D:
    var nearest: Node3D = null
    var nearest_distance := 34.0
    for part in parts:
        var projected := camera.unproject_position(part.global_position)
        var distance := projected.distance_to(screen_position)
        if distance < nearest_distance:
            nearest_distance = distance
            nearest = part
    return nearest

func _spawn_part(kind: String, position: Vector3, mark_prototype: bool) -> Node3D:
    var root := Node3D.new()
    root.name = "%s_%d" % [kind, parts.size() + 1]
    root.position = position
    root.set_meta("kind", kind)
    root.set_meta("prototype_only", mark_prototype)

    var body := MeshInstance3D.new()
    body.mesh = _mesh_for_kind(kind)
    var material := StandardMaterial3D.new()
    material.albedo_color = _color_for_kind(kind)
    material.metallic = 0.18
    material.roughness = 0.52
    body.material_override = material
    root.add_child(body)

    add_child(root)
    parts.append(root)
    return root

func _mesh_for_kind(kind: String) -> PrimitiveMesh:
    if kind == "LED":
        var sphere := SphereMesh.new()
        sphere.radius = 0.22
        sphere.height = 0.45
        return sphere
    if kind == "CHIP":
        var chip := BoxMesh.new()
        chip.size = Vector3(0.85, 0.20, 0.62)
        return chip
    if kind == "POWER":
        var power := BoxMesh.new()
        power.size = Vector3(0.85, 0.42, 0.72)
        return power
    if kind == "CAPACITOR":
        var capacitor := CylinderMesh.new()
        capacitor.top_radius = 0.22
        capacitor.bottom_radius = 0.22
        capacitor.height = 0.48
        return capacitor
    var resistor := BoxMesh.new()
    resistor.size = Vector3(0.82, 0.20, 0.28)
    return resistor

func _color_for_kind(kind: String) -> Color:
    match kind:
        "LED": return Color(0.25, 1.0, 0.18)
        "RESISTOR": return Color(0.72, 0.48, 0.20)
        "CAPACITOR": return Color(0.20, 0.55, 0.95)
        "CHIP": return Color(0.06, 0.08, 0.09)
        "POWER": return Color(0.88, 0.22, 0.18)
        _: return Color(0.75, 0.75, 0.75)

func _select_part(part: Node3D) -> void:
    selected_part = part
    var kind := str(part.get_meta("kind", "UNKNOWN"))
    _set_status("SELECTED %s — E PICKS SAME, CTRL+D DUPLICATES, SHIFT+ARROWS NUDGE." % kind)

func _pick_same_part() -> void:
    if selected_part == null:
        _set_status("PICK SAME NEEDS A SELECTED COMPONENT.")
        return
    selected_kind = str(selected_part.get_meta("kind", "LED"))
    _set_status("PICKED %s — CLICK AN OPEN GRID SPOT TO PLACE ANOTHER." % selected_kind)

func _duplicate_selected() -> void:
    if selected_part == null:
        _set_status("DUPLICATE NEEDS A SELECTED COMPONENT.")
        return
    var offsets := [Vector3(GRID_STEP, 0.0, 0.0), Vector3(-GRID_STEP, 0.0, 0.0), Vector3(0.0, 0.0, GRID_STEP), Vector3(0.0, 0.0, -GRID_STEP)]
    for offset in offsets:
        var candidate := selected_part.position + offset
        if _inside_board(candidate) and not _position_occupied(candidate):
            _record_history()
            var copy := _spawn_part(str(selected_part.get_meta("kind", "LED")), candidate, true)
            _select_part(copy)
            _set_status("DUPLICATED COMPONENT. WIRES ARE NOT COPIED. CTRL+Z UNDOS IT.")
            return
    _set_status("DUPLICATE BLOCKED — NO OPEN ADJACENT GRID SPOT.")

func _nudge_selected(keycode: Key) -> void:
    if selected_part == null:
        _set_status("NUDGE NEEDS A SELECTED COMPONENT.")
        return
    var delta := Vector3.ZERO
    match keycode:
        KEY_LEFT: delta.x = -GRID_STEP
        KEY_RIGHT: delta.x = GRID_STEP
        KEY_UP: delta.z = -GRID_STEP
        KEY_DOWN: delta.z = GRID_STEP
    var candidate := selected_part.position + delta
    if not _inside_board(candidate):
        _set_status("NUDGE BLOCKED — BOARD EDGE.")
        return
    if _position_occupied(candidate, selected_part):
        _set_status("NUDGE BLOCKED — ANOTHER COMPONENT IS IN THE WAY.")
        return
    _record_history()
    selected_part.position = candidate
    _set_status("NUDGED ONE GRID STEP. CTRL+Z UNDOS IT.")

func _record_history() -> void:
    var snapshot := {"parts": []}
    for part in parts:
        snapshot.parts.append({"kind": str(part.get_meta("kind", "LED")), "position": part.position})
    undo_stack.append(snapshot)
    if undo_stack.size() > HISTORY_LIMIT:
        undo_stack.pop_front()

func _undo() -> void:
    if undo_stack.is_empty():
        _set_status("NOTHING TO UNDO.")
        return
    var snapshot: Dictionary = undo_stack.pop_back()
    for part in parts:
        part.queue_free()
    parts.clear()
    selected_part = null
    for record in snapshot.parts:
        _spawn_part(record.kind, record.position, true)
    _set_status("UNDO — PROTOTYPE BOARD RESTORED.")

func _validate_with_core() -> void:
    var snapshot := {"parts": []}
    for part in parts:
        snapshot.parts.append({"kind": str(part.get_meta("kind", "UNKNOWN")), "position": part.position})
    var result := bridge.validate_board(snapshot)
    _set_status("VALIDATION: %s — %s" % [str(result.status).to_upper(), result.message])

func _set_status(message: String) -> void:
    if status_label != null:
        status_label.text = message
