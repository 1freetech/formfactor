extends "res://scripts/interactive_lab_1125.gd"

# FormFactor 1.126
# Three editor fixes:
# 1) undo/redo restores the current electronics-specific 3D packages,
# 2) moved wires keep the same visual height/thickness as newly made wires,
# 3) the live schematic mirrors actual player-created connections.

const SCHEMATIC_CANVAS_1126 = preload("res://scripts/schematic_symbol_canvas_1126.gd")
const WIRE_VISUAL_Y := 0.70
const WIRE_VISUAL_RADIUS := 0.035

func _ready() -> void:
    super._ready()
    if status_label != null:
        status_label.text = "Blank PCB ready. Build, move, wire, and undo without losing the current 3D component style."
    if build_status != null:
        build_status.text = "BUILD / BUY"

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.125"):
                label.text = "FORMFACTOR 1.126"
    var version_label := get_node_or_null("WorkbenchVersion3D") as Label3D
    if version_label != null:
        version_label.text = "1.126 // EDITOR FIXES"

func _install_vector_schematic() -> void:
    var hud := get_node_or_null("HUD")
    if hud == null:
        return
    var panel := hud.get_node_or_null("SchematicMirrorPanel") as ColorRect
    if panel == null:
        return
    if schematic_label != null:
        schematic_label.visible = false
    schematic_canvas = SCHEMATIC_CANVAS_1126.new()
    schematic_canvas.name = "VectorSchematicCanvas1126"
    schematic_canvas.position = Vector2(8, 38)
    schematic_canvas.size = Vector2(352, 364)
    schematic_canvas.mouse_filter = Control.MOUSE_FILTER_IGNORE
    panel.add_child(schematic_canvas)

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
    for wire_variant in user_wires:
        var wire := wire_variant as Dictionary
        var a := wire.get("a") as StaticBody3D
        var b := wire.get("b") as StaticBody3D
        if a == null or b == null or not is_instance_valid(a) or not is_instance_valid(b):
            continue
        connections.append({
            "a": str(a.get_meta("refdes", "?")),
            "b": str(b.get_meta("refdes", "?"))
        })

    if schematic_canvas.has_method("set_circuit_graph"):
        schematic_canvas.call("set_circuit_graph", rows, connections)
    else:
        schematic_canvas.call("set_circuit", rows, connections.size())

func _refresh_wires_for_component(body: StaticBody3D) -> void:
    for wire_variant in user_wires:
        var wire := wire_variant as Dictionary
        var a := wire.get("a") as StaticBody3D
        var b := wire.get("b") as StaticBody3D
        if a != body and b != body:
            continue

        var old_node := wire.get("node") as Node
        if old_node != null and is_instance_valid(old_node):
            old_node.queue_free()

        if a == null or b == null or not is_instance_valid(a) or not is_instance_valid(b):
            continue

        var start := a.position + Vector3(0.0, WIRE_VISUAL_Y, 0.0)
        var finish := b.position + Vector3(0.0, WIRE_VISUAL_Y, 0.0)
        var new_node := _segment(
            "UserWire_%d" % (user_wires.find(wire) + 1),
            start,
            finish,
            WIRE_VISUAL_RADIUS,
            _material(NORMAL_WIRE_COLOR, 0.52, 0.28)
        )
        new_node.set_meta("user_wire", true)
        wire["node"] = new_node

func _restore_board_snapshot(snapshot: Dictionary) -> void:
    # The older history layer used super._place_component_at_world(), which
    # bypassed newer component builders during replay. Rebuild through the
    # current runtime so restored parts keep the 1.125+ electronics geometry.
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

        # Call the current virtual placement path, not the historical parent
        # implementation. _build_component_visual therefore resolves to the
        # newest electronics-specific geometry.
        var body := _place_component_at_world(Vector3(
            float(state.get("x", 0.0)),
            float(state.get("y", BOARD_Y)),
            float(state.get("z", 0.0))))
        if body == null:
            continue

        body.rotation.y = float(state.get("rotation_y", 0.0))
        body.set_meta("refdes", saved_refdes)
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
            _connect_parts(a, b)

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

func debug_find_component(refdes: String) -> StaticBody3D:
    for body in placed_components:
        if body != null and is_instance_valid(body) and str(body.get_meta("refdes", "")) == refdes:
            return body
    return null

func debug_current_visual_present(refdes: String, node_name: String) -> bool:
    var body := debug_find_component(refdes)
    if body == null:
        return false
    return body.find_child(node_name, true, false) != null and not debug_component_visual_has_text(body)

func debug_connect_refs(a_ref: String, b_ref: String) -> bool:
    var a := debug_find_component(a_ref)
    var b := debug_find_component(b_ref)
    if a == null or b == null:
        return false
    _connect_parts(a, b)
    _refresh_schematic()
    return true

func debug_move_refdes(refdes: String, target: Vector3) -> bool:
    var body := debug_find_component(refdes)
    if body == null:
        return false
    var blocker := _component_near_position_except(target, body)
    if blocker != null:
        return false
    body.position = target
    _refresh_wires_for_component(body)
    _refresh_schematic()
    return true

func debug_wire_visuals_current() -> bool:
    for wire_variant in user_wires:
        var wire := wire_variant as Dictionary
        var node := wire.get("node") as MeshInstance3D
        if node == null or not is_instance_valid(node):
            return false
        if absf(node.position.y - WIRE_VISUAL_Y) > 0.001:
            return false
        var mesh := node.mesh as CylinderMesh
        if mesh == null:
            return false
        if absf(mesh.top_radius - WIRE_VISUAL_RADIUS) > 0.001:
            return false
        if absf(mesh.bottom_radius - WIRE_VISUAL_RADIUS) > 0.001:
            return false
    return true

func debug_schematic_connection_count() -> int:
    if schematic_canvas == null or not schematic_canvas.has_method("debug_connection_count"):
        return -1
    return int(schematic_canvas.call("debug_connection_count"))
