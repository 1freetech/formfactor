extends "res://scripts/interactive_lab_1126.gd"

# FormFactor 1.127 optimization layer.
# Rendering remains presentation-only; engineering truth stays in the C++ core.

const HISTORY_LIMIT_1127 := 96
const GRID_VISUAL_Y := 0.154
const GRID_LINE_WIDTH := 0.010

func _ready() -> void:
    super._ready()
    _install_1127_visuals()
    if status_label != null:
        status_label.text = "PCB ready. Place, move, duplicate, wire, test, undo, and repair."
    if build_status != null:
        build_status.text = "BUILD / BUY"

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.126"):
                label.text = "FORMFACTOR 1.127"
    var version_label := get_node_or_null("WorkbenchVersion3D") as Label3D
    if version_label != null:
        version_label.text = "1.127 // OPTIMIZED BUILD"

func _place_component_at_world(world_pos: Vector3) -> StaticBody3D:
    if world_pos == Vector3.INF:
        return null
    var blocker := _component_near_position_except(world_pos, null)
    if blocker != null:
        if status_label != null:
            status_label.text = "Placement blocked. Choose an open PCB grid position."
        if build_status != null:
            build_status.text = "BLOCKED // %s already occupies that grid area" % str(blocker.get_meta("refdes", "part"))
        return null

    var body := super._place_component_at_world(world_pos)
    if body != null:
        active_component = body
        _update_context_target(body)
    return body

func _commit_history_snapshot() -> bool:
    var changed := super._commit_history_snapshot()
    if not changed:
        return false

    while edit_history.size() > HISTORY_LIMIT_1127:
        edit_history.remove_at(0)
        history_index = maxi(0, history_index - 1)

    _update_history_controls()
    return true

func _unhandled_input(event: InputEvent) -> void:
    if event is InputEventKey:
        var key := event as InputEventKey
        if key.pressed and not key.echo:
            var focus_owner := get_viewport().gui_get_focus_owner()
            var text_editing := focus_owner is LineEdit or focus_owner is TextEdit
            if not text_editing:
                if key.keycode == KEY_DELETE or key.keycode == KEY_BACKSPACE:
                    _delete_active_component()
                    get_viewport().set_input_as_handled()
                    return
                if key.ctrl_pressed and key.keycode == KEY_D:
                    _duplicate_active_component()
                    get_viewport().set_input_as_handled()
                    return

    super._unhandled_input(event)

func _delete_active_component() -> bool:
    if active_component == null or not is_instance_valid(active_component):
        if build_status != null:
            build_status.text = "DELETE: click a placed component first"
        return false

    var target := active_component
    var refdes := str(target.get_meta("refdes", "part"))
    var component_id := str(target.get_meta("component_id", ""))

    inventory_stock[component_id] = int(inventory_stock.get(component_id, 0)) + 1
    _remove_wires_for(target)
    placed_components.erase(target)

    if hovered_component == target:
        hovered_component = null
    active_component = null
    dragging_component = null
    target.queue_free()

    _reset_player_test_visuals()
    _refresh_inventory()
    _refresh_schematic()
    _update_selected_label()
    _update_context_target(null)
    if focus_marker != null:
        focus_marker.visible = false

    if status_label != null:
        status_label.text = "%s removed. Ctrl+Z restores the board state." % refdes
    if build_status != null:
        build_status.text = "REMOVED // %s" % refdes

    _commit_history_snapshot()
    return true

func _duplicate_active_component() -> bool:
    if active_component == null or not is_instance_valid(active_component):
        if build_status != null:
            build_status.text = "DUPLICATE: click a placed component first"
        return false

    var source := active_component
    var component_id := str(source.get_meta("component_id", ""))
    if int(inventory_stock.get(component_id, 0)) <= 0:
        if build_status != null:
            build_status.text = "DUPLICATE: no inventory stock remains"
        return false

    var previous_selection := selected_component_id
    for radius in range(1, 9):
        for dz in range(-radius, radius + 1):
            for dx in range(-radius, radius + 1):
                if maxi(abs(dx), abs(dz)) != radius:
                    continue

                var candidate := source.position + Vector3(float(dx) * GRID_STEP, 0.0, float(dz) * GRID_STEP)
                candidate.y = BOARD_Y
                if absf(candidate.x) > BOARD_X_LIMIT or absf(candidate.z) > BOARD_Z_LIMIT:
                    continue
                if _component_near_position_except(candidate, null) != null:
                    continue

                selected_component_id = component_id
                var duplicate := super._place_component_at_world(candidate)
                selected_component_id = previous_selection
                _update_selected_label()
                if duplicate == null:
                    continue

                duplicate.rotation.y = source.rotation.y
                active_component = duplicate
                _update_context_target(duplicate)
                _reset_player_test_visuals()
                _refresh_schematic()

                if status_label != null:
                    status_label.text = "%s duplicated as %s. Wires are not copied." % [
                        str(source.get_meta("refdes", "part")),
                        str(duplicate.get_meta("refdes", "part"))
                    ]
                if build_status != null:
                    build_status.text = "DUPLICATED // %s" % str(duplicate.get_meta("refdes", "part"))

                _commit_history_snapshot()
                return true

    selected_component_id = previous_selection
    _update_selected_label()
    if build_status != null:
        build_status.text = "DUPLICATE BLOCKED // no open grid position nearby"
    return false

func _install_1127_visuals() -> void:
    if get_node_or_null("PCBPlacementGrid1127") == null:
        var grid_root := Node3D.new()
        grid_root.name = "PCBPlacementGrid1127"
        add_child(grid_root)

        var grid_material := _material(Color(0.34, 0.88, 0.58, 0.16), 0.0, 0.76)
        grid_material.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
        grid_material.emission_enabled = true
        grid_material.emission = Color(0.08, 0.42, 0.20, 1.0)
        grid_material.emission_energy_multiplier = 0.22

        var x := -4.0
        var x_index := 0
        while x <= 4.001:
            _box_child(
                grid_root,
                "GridX_%02d" % x_index,
                Vector3(GRID_LINE_WIDTH, 0.006, 4.82),
                Vector3(x, GRID_VISUAL_Y, 0.0),
                grid_material
            )
            x += GRID_STEP
            x_index += 1

        var z := -2.0
        var z_index := 0
        while z <= 2.001:
            _box_child(
                grid_root,
                "GridZ_%02d" % z_index,
                Vector3(8.10, 0.006, GRID_LINE_WIDTH),
                Vector3(0.0, GRID_VISUAL_Y, z),
                grid_material
            )
            z += GRID_STEP
            z_index += 1

    if get_node_or_null("PCBVisualUnderlay1127") == null:
        _box(
            "PCBVisualUnderlay1127",
            Vector3(8.68, 0.045, 5.33),
            Vector3(0.0, -0.083, 0.0),
            _material(Color(0.16, 0.09, 0.045, 1.0), 0.02, 0.68)
        )

    if get_node_or_null("SoftRimLight1127") == null:
        var rim := OmniLight3D.new()
        rim.name = "SoftRimLight1127"
        rim.position = Vector3(4.2, 3.1, 2.9)
        rim.light_color = Color(0.72, 0.86, 1.0, 1.0)
        rim.light_energy = 1.55
        rim.omni_range = 8.5
        rim.shadow_enabled = false
        add_child(rim)

func debug_1127_visuals_ready() -> bool:
    var grid_ready := get_node_or_null("PCBPlacementGrid1127") != null
    var depth_ready := get_node_or_null("PCBVisualUnderlay1127") != null
    var light_ready := get_node_or_null("SoftRimLight1127") != null
    return grid_ready and depth_ready and light_ready

func debug_history_limit_1127() -> int:
    return HISTORY_LIMIT_1127

func debug_set_active_refdes_1127(refdes: String) -> bool:
    var body := debug_find_component(refdes)
    if body == null:
        return false
    active_component = body
    _update_context_target(body)
    return true

func debug_duplicate_active_1127() -> bool:
    return _duplicate_active_component()

func debug_delete_active_1127() -> bool:
    return _delete_active_component()
