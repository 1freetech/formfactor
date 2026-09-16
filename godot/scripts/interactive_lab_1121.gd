extends "res://scripts/interactive_lab_111.gd"

const HISTORY_SETTLE_SECONDS := 0.16
const FOCUS_TWEEN_SECONDS := 0.32
const BOARD_CAMERA_POSITION := Vector3(0.0, 4.8, 8.4)
const INSPECT_CAMERA_POSITION := Vector3(0.0, 3.55, 5.9)

var context_panel: ColorRect
var context_title: Label
var context_detail: Label
var history_label: Label
var undo_button: Button
var redo_button: Button
var focus_marker: MeshInstance3D
var hovered_component: StaticBody3D = null
var last_focus_target := Vector3.ZERO

var edit_history: Array[Dictionary] = []
var history_index := -1
var history_replaying := false
var history_signature := ""
var observed_signature := ""
var history_settle_timer := 0.0

func _ready() -> void:
    super._ready()
    _build_focus_marker()
    _initialize_edit_history()
    _update_context_target(null)

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.11"):
                label.text = "FORMFACTOR 1.121 // SMART INTERACTION BUILD LAB"
    _label3d("1.121 // FOCUS • UNDO • REDO • BUILD", Vector3(-1.72, 0.19, -1.82), 0.0068, NEON)

func _build_gameplay_hud() -> void:
    super._build_gameplay_hud()
    var hud := get_node_or_null("HUD") as CanvasLayer
    if hud == null:
        return

    context_panel = ColorRect.new()
    context_panel.name = "SmartTargetPanel"
    context_panel.position = Vector2(800, 18)
    context_panel.size = Vector2(198, 82)
    context_panel.color = Color(0.010, 0.022, 0.028, 0.94)
    hud.add_child(context_panel)

    context_title = Label.new()
    context_title.name = "SmartTargetTitle"
    context_title.position = Vector2(10, 8)
    context_title.size = Vector2(178, 22)
    context_title.text = "SMART TARGET"
    context_title.add_theme_color_override("font_color", NEON)
    context_title.add_theme_font_size_override("font_size", 13)
    context_panel.add_child(context_title)

    context_detail = Label.new()
    context_detail.name = "SmartTargetDetail"
    context_detail.position = Vector2(10, 31)
    context_detail.size = Vector2(178, 43)
    context_detail.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    context_detail.add_theme_color_override("font_color", Color(0.80, 0.88, 0.90, 1.0))
    context_detail.add_theme_font_size_override("font_size", 11)
    context_panel.add_child(context_detail)

    undo_button = Button.new()
    undo_button.name = "UndoBoardButton"
    undo_button.text = "UNDO"
    undo_button.position = Vector2(804, 112)
    undo_button.size = Vector2(66, 40)
    undo_button.tooltip_text = "Undo the last player-board edit (Ctrl+Z)"
    undo_button.pressed.connect(_undo_board_edit)
    hud.add_child(undo_button)

    redo_button = Button.new()
    redo_button.name = "RedoBoardButton"
    redo_button.text = "REDO"
    redo_button.position = Vector2(874, 112)
    redo_button.size = Vector2(66, 40)
    redo_button.tooltip_text = "Redo an undone player-board edit (Ctrl+Y)"
    redo_button.pressed.connect(_redo_board_edit)
    hud.add_child(redo_button)

    var focus_button := Button.new()
    focus_button.name = "FocusTargetButton"
    focus_button.text = "FOCUS"
    focus_button.position = Vector2(944, 112)
    focus_button.size = Vector2(68, 40)
    focus_button.tooltip_text = "Smoothly focus the active or hovered board component (F)"
    focus_button.pressed.connect(_focus_best_target)
    hud.add_child(focus_button)

    var board_button := Button.new()
    board_button.name = "FocusBoardButton"
    board_button.text = "BOARD"
    board_button.position = Vector2(1016, 112)
    board_button.size = Vector2(68, 40)
    board_button.tooltip_text = "Return to the full PCB workbench view (B)"
    board_button.pressed.connect(_focus_board)
    hud.add_child(board_button)

    history_label = Label.new()
    history_label.name = "BoardHistoryStatus"
    history_label.position = Vector2(804, 96)
    history_label.size = Vector2(280, 16)
    history_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
    history_label.add_theme_color_override("font_color", Color(0.62, 0.73, 0.75, 1.0))
    history_label.add_theme_font_size_override("font_size", 10)
    hud.add_child(history_label)

func _build_focus_marker() -> void:
    focus_marker = MeshInstance3D.new()
    focus_marker.name = "SmartTargetMarker3D"
    var mesh := CylinderMesh.new()
    mesh.top_radius = 0.44
    mesh.bottom_radius = 0.44
    mesh.height = 0.018
    mesh.radial_segments = 40
    focus_marker.mesh = mesh
    var marker_material := StandardMaterial3D.new()
    marker_material.albedo_color = Color(0.12, 0.62, 0.08, 0.68)
    marker_material.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
    marker_material.emission_enabled = true
    marker_material.emission = NEON
    marker_material.emission_energy_multiplier = 2.0
    marker_material.roughness = 0.28
    focus_marker.material_override = marker_material
    focus_marker.visible = false
    add_child(focus_marker)

func _process(delta_seconds: float) -> void:
    super._process(delta_seconds)
    _update_hover_target()
    _track_edit_history(delta_seconds)
    if focus_marker != null and focus_marker.visible:
        var pulse := 1.0 + sin(float(Time.get_ticks_msec()) * 0.008) * 0.055
        focus_marker.scale = Vector3(pulse, 1.0, pulse)

func _unhandled_input(event: InputEvent) -> void:
    if event is InputEventKey:
        var key := event as InputEventKey
        if key.pressed and not key.echo:
            var focus_owner := get_viewport().gui_get_focus_owner()
            var text_editing := focus_owner is LineEdit or focus_owner is TextEdit
            if not text_editing:
                if key.ctrl_pressed and key.keycode == KEY_Z:
                    if key.shift_pressed:
                        _redo_board_edit()
                    else:
                        _undo_board_edit()
                    get_viewport().set_input_as_handled()
                    return
                if key.ctrl_pressed and key.keycode == KEY_Y:
                    _redo_board_edit()
                    get_viewport().set_input_as_handled()
                    return
                if key.keycode == KEY_F:
                    _focus_best_target()
                    get_viewport().set_input_as_handled()
                    return
                if key.keycode == KEY_B:
                    _focus_board()
                    get_viewport().set_input_as_handled()
                    return
    super._unhandled_input(event)

func _update_hover_target() -> void:
    if camera == null:
        return
    var collider := _ray_collider(get_viewport().get_mouse_position())
    var next_hover: StaticBody3D = null
    if collider != null and bool(collider.get_meta("placed_component", false)):
        next_hover = collider as StaticBody3D
    if next_hover != hovered_component:
        hovered_component = next_hover
        _update_context_target(hovered_component)
    if focus_marker == null:
        return
    if hovered_component != null and is_instance_valid(hovered_component):
        focus_marker.visible = true
        focus_marker.position = Vector3(hovered_component.position.x, BOARD_Y + 0.015, hovered_component.position.z)
    elif active_component != null and is_instance_valid(active_component):
        focus_marker.visible = true
        focus_marker.position = Vector3(active_component.position.x, BOARD_Y + 0.015, active_component.position.z)
    else:
        focus_marker.visible = false

func _update_context_target(target: StaticBody3D) -> void:
    if context_detail == null:
        return
    var use_target := target
    if use_target == null and active_component != null and is_instance_valid(active_component):
        use_target = active_component
    if use_target == null or not is_instance_valid(use_target):
        context_detail.text = "%s\nF focus target • B full board" % _current_workspace_zone()
        return
    var component_id := str(use_target.get_meta("component_id", ""))
    var refdes := str(use_target.get_meta("refdes", "part"))
    var component := _component_by_id(component_id)
    var name := str(component.get("name", component_id))
    context_detail.text = "%s // %s\nF focus • click/select • W wire" % [refdes, name]

func _current_workspace_zone() -> String:
    if cad_panel != null and cad_panel.visible:
        return "ZONE // PARAMETRIC CAD"
    if tutorial_panel != null and tutorial_panel.visible:
        return "ZONE // ACADEMY"
    return "ZONE // PCB BENCH"

func _focus_best_target() -> void:
    var target := active_component
    if target == null or not is_instance_valid(target):
        target = hovered_component
    if target == null or not is_instance_valid(target):
        _focus_board()
        return
    active_component = target
    var target_position := Vector3(target.position.x, 0.0, target.position.z)
    last_focus_target = target_position
    _animate_camera_focus(target_position, INSPECT_CAMERA_POSITION)
    _update_context_target(target)
    if build_status != null:
        build_status.text = "FOCUS: %s" % str(target.get_meta("refdes", "part"))

func _focus_board() -> void:
    last_focus_target = Vector3.ZERO
    _animate_camera_focus(Vector3.ZERO, BOARD_CAMERA_POSITION)
    if build_status != null:
        build_status.text = "VIEW: full PCB workbench"

func _animate_camera_focus(pivot_target: Vector3, camera_target: Vector3) -> void:
    if camera_pivot == null or camera == null:
        return
    var tween := create_tween()
    tween.set_parallel(true)
    tween.set_trans(Tween.TRANS_QUAD)
    tween.set_ease(Tween.EASE_OUT)
    tween.tween_property(camera_pivot, "position", pivot_target, FOCUS_TWEEN_SECONDS)
    tween.tween_property(camera, "position", camera_target, FOCUS_TWEEN_SECONDS)

func _initialize_edit_history() -> void:
    edit_history.clear()
    var snapshot := _capture_board_snapshot()
    edit_history.append(snapshot)
    history_index = 0
    history_signature = JSON.stringify(snapshot)
    observed_signature = history_signature
    history_settle_timer = 0.0
    _update_history_controls()

func _track_edit_history(delta_seconds: float) -> void:
    if history_replaying or history_index < 0:
        return
    var current_signature := _board_signature()
    if current_signature != observed_signature:
        observed_signature = current_signature
        history_settle_timer = 0.0
        return
    if current_signature == history_signature:
        history_settle_timer = 0.0
        return
    history_settle_timer += delta_seconds
    if history_settle_timer >= HISTORY_SETTLE_SECONDS:
        _commit_history_snapshot()

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
            "b": str(b.get_meta("refdes", "?"))
        })
    return {"components": components, "wires": wires}

func _board_signature() -> String:
    return JSON.stringify(_capture_board_snapshot())

func _commit_history_snapshot() -> bool:
    if history_replaying:
        return false
    var snapshot := _capture_board_snapshot()
    var signature := JSON.stringify(snapshot)
    if signature == history_signature:
        observed_signature = signature
        history_settle_timer = 0.0
        return false
    if history_index < edit_history.size() - 1:
        edit_history.resize(history_index + 1)
    edit_history.append(snapshot)
    history_index = edit_history.size() - 1
    history_signature = signature
    observed_signature = signature
    history_settle_timer = 0.0
    _update_history_controls()
    return true

func _undo_board_edit() -> void:
    if history_replaying:
        return
    if _board_signature() != history_signature:
        _commit_history_snapshot()
    if history_index <= 0:
        if build_status != null:
            build_status.text = "UNDO: already at the first board state"
        return
    history_index -= 1
    _restore_board_snapshot(edit_history[history_index])
    if build_status != null:
        build_status.text = "UNDO: restored board state %d/%d" % [history_index + 1, edit_history.size()]

func _redo_board_edit() -> void:
    if history_replaying:
        return
    if _board_signature() != history_signature:
        _commit_history_snapshot()
    if history_index >= edit_history.size() - 1:
        if build_status != null:
            build_status.text = "REDO: no newer board state"
        return
    history_index += 1
    _restore_board_snapshot(edit_history[history_index])
    if build_status != null:
        build_status.text = "REDO: restored board state %d/%d" % [history_index + 1, edit_history.size()]

func _restore_board_snapshot(snapshot: Dictionary) -> void:
    history_replaying = true
    hovered_component = null
    active_component = null
    dragging_component = null
    super._clear_player_board()

    reference_counts.clear()
    for component in COMPONENTS:
        var prefix := str(component.get("prefix", "X"))
        reference_counts[prefix] = 0

    var bodies_by_refdes: Dictionary = {}
    var component_states: Array = snapshot.get("components", [])
    for state_variant in component_states:
        var state := state_variant as Dictionary
        var component_id := str(state.get("component_id", ""))
        var saved_refdes := str(state.get("refdes", "part"))
        selected_component_id = component_id
        var body := super._place_component_at_world(Vector3(
            float(state.get("x", 0.0)),
            float(state.get("y", BOARD_Y)),
            float(state.get("z", 0.0))))
        if body == null:
            continue
        body.rotation.y = float(state.get("rotation_y", 0.0))
        body.set_meta("refdes", saved_refdes)
        for child in body.find_children("*", "Label3D", true, false):
            var label := child as Label3D
            if label != null:
                label.text = saved_refdes
        bodies_by_refdes[saved_refdes] = body
        var component := _component_by_id(component_id)
        var prefix := str(component.get("prefix", "X"))
        if saved_refdes.begins_with(prefix):
            var number := saved_refdes.substr(prefix.length()).to_int()
            reference_counts[prefix] = maxi(int(reference_counts.get(prefix, 0)), number)

    var wire_states: Array = snapshot.get("wires", [])
    for wire_variant in wire_states:
        var wire_state := wire_variant as Dictionary
        var a_ref := str(wire_state.get("a", ""))
        var b_ref := str(wire_state.get("b", ""))
        var a := bodies_by_refdes.get(a_ref) as StaticBody3D
        var b := bodies_by_refdes.get(b_ref) as StaticBody3D
        if a != null and b != null:
            super._connect_parts(a, b)

    selected_component_id = ""
    wire_mode = false
    wire_start = null
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

func _update_history_controls() -> void:
    if undo_button != null:
        undo_button.disabled = history_index <= 0
    if redo_button != null:
        redo_button.disabled = history_index < 0 or history_index >= edit_history.size() - 1
    if history_label != null:
        history_label.text = "BOARD HISTORY %d/%d • Ctrl+Z / Ctrl+Y" % [history_index + 1, edit_history.size()]

func debug_context_panel_ready() -> bool:
    return context_panel != null and context_title != null and context_detail != null

func debug_focus_marker_ready() -> bool:
    return focus_marker != null

func debug_history_count() -> int:
    return edit_history.size()

func debug_history_index() -> int:
    return history_index

func debug_force_history_commit() -> bool:
    return _commit_history_snapshot()

func debug_undo_board_edit() -> bool:
    var before := history_index
    _undo_board_edit()
    return history_index < before

func debug_redo_board_edit() -> bool:
    var before := history_index
    _redo_board_edit()
    return history_index > before

func debug_component_position(refdes: String) -> Vector3:
    for body in placed_components:
        if body != null and is_instance_valid(body) and str(body.get_meta("refdes", "")) == refdes:
            return body.position
    return Vector3.INF

func debug_set_active_by_refdes(refdes: String) -> bool:
    for body in placed_components:
        if body != null and is_instance_valid(body) and str(body.get_meta("refdes", "")) == refdes:
            active_component = body
            _update_context_target(body)
            return true
    return false

func debug_focus_active() -> bool:
    if active_component == null or not is_instance_valid(active_component):
        return false
    _focus_best_target()
    return true

func debug_last_focus_target() -> Vector3:
    return last_focus_target

func debug_workspace_zone() -> String:
    return _current_workspace_zone()
