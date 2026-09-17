extends "res://scripts/interactive_lab_1121.gd"

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.121"):
                label.text = "FORMFACTOR 1.122 // GLITCH-FIX BUILD LAB"
    for child in find_children("*", "Label3D", true, false):
        var label3d := child as Label3D
        if label3d != null and label3d.text.begins_with("1.121 //"):
            label3d.text = "1.122 // FOCUS • CLEAN DRAG UNDO • BUILD"

func _focus_best_target() -> void:
    # The bright hover marker is the player's strongest visual cue, so the
    # component under that marker must win over an older selected component.
    var target := hovered_component
    if target == null or not is_instance_valid(target):
        target = active_component
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

func _track_edit_history(delta_seconds: float) -> void:
    # Never record intermediate drag positions. A move becomes one undo step
    # only after the player releases the component and the board settles.
    if dragging_component != null and is_instance_valid(dragging_component):
        history_settle_timer = 0.0
        return
    super._track_edit_history(delta_seconds)

func debug_set_hover_by_refdes(refdes: String) -> bool:
    for body in placed_components:
        if body != null and is_instance_valid(body) and str(body.get_meta("refdes", "")) == refdes:
            hovered_component = body
            _update_context_target(body)
            return true
    hovered_component = null
    _update_context_target(null)
    return false

func debug_focus_best_target() -> bool:
    var before := last_focus_target
    _focus_best_target()
    return last_focus_target != before or last_focus_target != Vector3.ZERO

func debug_set_dragging_by_refdes(refdes: String) -> bool:
    for body in placed_components:
        if body != null and is_instance_valid(body) and str(body.get_meta("refdes", "")) == refdes:
            dragging_component = body
            dragging_original_position = body.position
            return true
    return false

func debug_move_dragging_to(world_position: Vector3) -> bool:
    if dragging_component == null or not is_instance_valid(dragging_component):
        return false
    dragging_component.position = world_position
    _refresh_wires_for_component(dragging_component)
    return true

func debug_clear_dragging() -> void:
    dragging_component = null

func debug_track_history(delta_seconds: float) -> void:
    _track_edit_history(delta_seconds)
