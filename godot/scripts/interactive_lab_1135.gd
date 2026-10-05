extends "res://scripts/interactive_lab_1134.gd"

# Wordless construction UX. Geometric fit is NOT electrical or manufacturing approval.
# Teaching meshes remain generic; unsupported engineering results stay UNKNOWN.
const BUILD_GRID := 0.254 # 2.54 mm placement grid; not a claimed footprint.
const FIT_GAP := 0.08 # conservative visual separation, not a DRC rule.
const ICON_PATHS := {
    "resistor": "M4 16h5l3-5 4 10 4-10 3 5h5",
    "potentiometer": "M4 16h5l3-5 4 10 4-10 3 5h5 M22 4l-8 8m0-5v5h5",
    "ceramic_cap": "M4 16h9m0-8v16m6-16v16m0-8h9",
    "electrolytic_cap": "M4 16h9m0-8v16m7-16q-5 8 0 16m0-8h8 M5 5h6m-3-3v6",
    "inductor": "M3 16h4q0-12 5-12q5 0 5 12q0-12 5-12q5 0 5 12h3",
    "diode": "M3 16h7m0-7v14l12-7z M22 8v16m0-8h7",
    "zener": "M3 16h7m0-7v14l12-7z M19 8h3v16h3m-3-8h7",
    "led": "M3 20h7m0-6v12l10-6z M20 13v14m0-7h7 M17 9l5-5m-4 0h4v4 M24 12l5-5m-4 0h4v4",
    "npn": "M4 16h8m0-8v16m0-10 10-7m-10 11 10 7m-5-1h5v-5",
    "pnp": "M4 16h8m0-8v16m0-10 10-7m-10 11 10 7m-5-1v-5h5",
    "nmos": "M3 16h7m0-8v16m5-16v16m0-16h10v-5m-10 21h10v5 M21 12l-4 4 4 4",
    "pmos": "M3 16h7m0-8v16m5-16v16m0-16h10v-5m-10 21h10v5 M17 12l4 4-4 4",
    "logic": "M4 16h5m0-9v18l12-9z M21 16a3 3 0 1 0 6 0a3 3 0 1 0-6 0m6 0h3",
    "opamp": "M3 11h7m-7 10h7m0-15v20l15-10z M25 16h5 M12 11h5m-2-2v4 M12 21h5",
    "comparator": "M3 11h7m-7 10h7m0-15v20l15-10z M25 16h5 M12 11h5m-2-2v4 M12 21h5 M26 8v3h4",
    "regulator": "M8 8h16v16H8z M2 16h6m16 0h6 M16 24v6",
    "fuse": "M3 16h5m0-6h16v12H8z M8 16h16m0 0h5",
    "switch": "M3 22h7m0 0 13-13 M23 22h6 M10 22a1 1 0 1 0 2 0 M21 22a1 1 0 1 0 2 0",
    "relay": "M3 5h10v22H3z M8 8v16 M18 24v-5l10-10m0 15v-6",
    "connector": "M10 4h12v24H10z M3 8h7m-7 8h7m-7 8h7 M14 8h4m-4 8h4m-4 8h4",
    "test_point": "M3 24h26 M16 24V12 M11 7a5 5 0 1 0 10 0a5 5 0 1 0-10 0",
    "sensor": "M6 6h20v20H6z M11 16q5-12 10 0q-5 12-10 0 M16 2v4m0 20v4",
    "buzzer": "M4 11h7l7-6v22l-7-6H4z M22 10q8 6 0 12m4-16q12 10 0 20",
    "power": "M3 16h9m0-9v18m7-13v8m0-4h10 M7 4h6m-3-3v6",
    "ground": "M16 3v12 M4 15h24 M8 21h16 M12 27h8",
    "catalog": "M4 4h9v9H4z M19 4h9v9h-9z M4 19h9v9H4z M19 19h9v9h-9z",
    "pointer": "M7 4v23l6-6 5 8 4-3-5-8h10z",
    "wire": "M4 6h10v20h14 M1 3h6v6H1z M25 23h6v6h-6z",
    "undo": "M11 5l-8 8 8 8 M3 13h15q12 0 12 14",
    "redo": "M21 5l8 8-8 8 M29 13H14Q2 13 2 27",
    "rotate": "M7 8a12 12 0 1 1-2 14 M7 2v8H0",
    "board": "M3 7h26v18H3z M8 12h1m14 0h1m-16 8h1m14 0h1",
    "help": "M9 9q0-10 12-5q8 6-5 12v4 M16 26v2",
    "inspect": "M12 4a8 8 0 1 0 0 16a8 8 0 1 0 0-16 M18 18l11 11",
    "test": "M4 16h5l4-10 6 20 4-10h5",
    "schematic": "M2 7h10v7h8v11h10 M8 3v8m16 10v8",
    "cad": "M3 6h26v20H3z M8 11h16v10H8z M12 3v6m8-6v6 M12 23v6m8-6v6",
    "learn": "M16 8Q8 1 2 5v22q7-5 14 0q7-5 14 0V5q-7-4-14 3v19",
    "view": "M16 3 29 10v13l-13 7-13-7V10z M3 10l13 7 13-7 M16 17v13"
}
var smooth_hud: Control
var catalog_grid: GridContainer
var catalog_panel: PanelContainer
var help_panel: PanelContainer
var help_text: RichTextLabel
var tool_buttons: Dictionary = {}
var footprint_cache: Dictionary = {}
var placement_preview: Node3D
var preview_id := ""
var preview_target := Vector3.INF
var preview_valid := false
var engineering_state := "UNKNOWN"
var connectivity_found := false

func _ready() -> void:
    super._ready()
    # Retire the oversized historical decorative layers; retain dimensioned substrate.
    pbr_board_details.visible = false
    guides_visible = false
    board_guide.visible = false
    _install_smooth_hud()
    _hide_world_text()
    _refresh_inventory()

func _hide_world_text() -> void:
    for label in find_children("*", "Label3D", true, false):
        label.visible = false

func _icon(id: String) -> Texture2D:
    var svg := '<svg xmlns="http://www.w3.org/2000/svg" width="40" height="40" viewBox="-3 -3 38 38"><path d="%s" fill="none" stroke="#a9eed5" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/></svg>' % ICON_PATHS.get(id, ICON_PATHS.board)
    var image := Image.new()
    image.load_svg_from_string(svg)
    return ImageTexture.create_from_image(image)

func _icon_button(id: String, action: Callable) -> Button:
    var button := Button.new()
    button.name = "Icon_" + id
    button.icon = _icon(id)
    button.custom_minimum_size = Vector2(48, 48)
    button.expand_icon = true
    button.focus_mode = Control.FOCUS_NONE
    button.pressed.connect(action)
    return button

func _install_smooth_hud() -> void:
    var hud := get_node("HUD")
    for child in hud.get_children():
        if child is CanvasItem:
            child.visible = false
    inspection_hud_visible = false
    inspection_toolbar_visible = false
    layer_details_visible = false
    schematic_mirror_visible = false
    smooth_hud = Control.new()
    smooth_hud.name = "SmoothBuildHUD"
    smooth_hud.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
    smooth_hud.mouse_filter = Control.MOUSE_FILTER_IGNORE
    hud.add_child(smooth_hud)
    var dock := HBoxContainer.new()
    dock.set_anchors_and_offsets_preset(Control.PRESET_CENTER_BOTTOM)
    dock.position = Vector2(-364, -66)
    dock.add_theme_constant_override("separation", 4)
    smooth_hud.add_child(dock)
    var actions := {
        "catalog": _toggle_catalog, "pointer": _cancel_build,
        "wire": _toggle_wire_mode, "undo": _undo_board_edit,
        "redo": _redo_board_edit, "rotate": _rotate_selected,
        "board": _reset_inspection_presentation, "view": _cycle_inspection_view,
        "schematic": _toggle_schematic_mirror, "inspect": _toggle_details,
        "test": _test_player_circuit, "cad": _toggle_cad_panel,
        "learn": _toggle_tutorial_panel, "help": _toggle_inspection_help
    }
    for id in actions:
        var button := _icon_button(id, actions[id])
        dock.add_child(button)
        tool_buttons[id] = button
    catalog_panel = PanelContainer.new()
    catalog_panel.position = Vector2(16, 260)
    smooth_hud.add_child(catalog_panel)
    catalog_grid = GridContainer.new()
    catalog_grid.columns = 5
    catalog_panel.add_child(catalog_grid)
    help_panel = PanelContainer.new()
    help_panel.position = Vector2(1090, 150)
    help_panel.size = Vector2(370, 440)
    help_panel.visible = false
    smooth_hud.add_child(help_panel)
    help_text = RichTextLabel.new()
    help_text.custom_minimum_size = Vector2(350, 420)
    help_text.bbcode_enabled = true
    help_panel.add_child(help_text)
    _refresh_help()

func _refresh_help() -> void:
    if help_text == null:
        return
    var part := _component_by_id(selected_component_id)
    help_text.text = "[b]BUILD / HELP[/b]\nChoose a symbol, then click or drag it onto the PCB. Keep clicking to place more. Escape or the pointer cancels. Delete removes the hovered part. Drag placed parts to move them. Q/E rotates; Ctrl+Z/Y undoes/redoes. Empty-space drag orbits; wheel zooms.\n\nDock, left to right: catalog, select, wire, undo, redo, rotate, board, view, schematic, details, connectivity check, CAD, lessons, help.\n\nSelected: %s\n\n[b]Engineering: %s[/b]\nThe check reports component-level connectivity only. It does not verify pin polarity, return paths, current limits, voltage, thermal behavior or manufacturing rules. A visible wire is not proof of a safe circuit. Generic meshes and visual fit are not certified footprints.\n\nConnectivity path found: %s\n%s" % [part.get("name", "none"), engineering_state, str(connectivity_found), status_label.text]

func _refresh_inventory() -> void:
    super._refresh_inventory()
    if catalog_grid == null:
        return
    for child in catalog_grid.get_children():
        catalog_grid.remove_child(child)
        child.queue_free()
    for component in COMPONENTS:
        var id := str(component.id)
        var button := _icon_button(id, select_component.bind(id))
        button.disabled = int(inventory_stock.get(id, 0)) <= 0
        button.modulate = Color(0.45, 0.9, 1.0) if selected_component_id == id else Color.WHITE
        button.gui_input.connect(_catalog_tile_gui_input.bind(id))
        catalog_grid.add_child(button)
    _refresh_help()

func select_component(component_id: String) -> void:
    super.select_component(component_id)
    if catalog_drag_ghost != null:
        catalog_drag_ghost.visible = false
    _refresh_help()

func _toggle_catalog() -> void:
    catalog_panel.visible = not catalog_panel.visible

func _toggle_inspection_help() -> void:
    help_panel.visible = not help_panel.visible
    _refresh_help()

func _toggle_inspection_toolbar() -> void:
    # C used to clear the board before reaching the controls handler. The new
    # handler below consumes C first; this method is safe and non-destructive.
    _toggle_catalog()

func _toggle_details() -> void:
    lexicon_panel.visible = not lexicon_panel.visible

func _cancel_build() -> void:
    selected_component_id = ""
    wire_mode = false
    wire_start = null
    _reset_catalog_drag_state()
    _refresh_inventory()
    _refresh_preview_id()

func _rotate_selected() -> void:
    rotate_active_component(90.0)

func _show_lexicon(component_id: String) -> void:
    super._show_lexicon(component_id)
    # Selection never forces open a word-heavy inspector.
    if smooth_hud != null:
        lexicon_panel.visible = false

func _screen_to_board(screen_pos: Vector2) -> Vector3:
    var origin := camera.project_ray_origin(screen_pos)
    var direction := camera.project_ray_normal(screen_pos)
    if absf(direction.y) < 0.0001:
        return Vector3.INF
    var distance := (BOARD_TOP_Y - origin.y) / direction.y
    if distance <= 0.0:
        return Vector3.INF
    var hit := origin + direction * distance
    if absf(hit.x) > mm(BOARD_WIDTH_MM) * 0.5 or absf(hit.z) > mm(BOARD_DEPTH_MM) * 0.5:
        return Vector3.INF
    return Vector3(snappedf(hit.x, BUILD_GRID), BOARD_TOP_Y, snappedf(hit.z, BUILD_GRID))

func _visual_half_size(id: String) -> Vector2:
    if footprint_cache.has(id):
        return footprint_cache[id]
    var probe := StaticBody3D.new()
    _build_component_visual(probe, _component_by_id(id), "")
    var half := Vector2(0.36, 0.31) # includes the inherited selection envelope.
    for child in probe.get_children():
        if child is MeshInstance3D and child.mesh != null:
            var bounds: AABB = child.mesh.get_aabb()
            for x in [bounds.position.x, bounds.end.x]:
                for y in [bounds.position.y, bounds.end.y]:
                    for z in [bounds.position.z, bounds.end.z]:
                        var p: Vector3 = child.transform * Vector3(x, y, z)
                        half.x = maxf(half.x, absf(p.x))
                        half.y = maxf(half.y, absf(p.z))
    probe.free()
    footprint_cache[id] = half
    return half

func _rotated_half(id: String, angle: float) -> Vector2:
    var half := _visual_half_size(id)
    return Vector2(absf(cos(angle)) * half.x + absf(sin(angle)) * half.y,
        absf(sin(angle)) * half.x + absf(cos(angle)) * half.y)

func _fit_at(id: String, pos: Vector3, ignored: StaticBody3D = null, angle: float = 0.0) -> bool:
    if id.is_empty() or not pos.is_finite() or absf(pos.y - BOARD_TOP_Y) > 0.001:
        return false
    var half := _rotated_half(id, angle)
    if absf(pos.x) + half.x + FIT_GAP > mm(BOARD_WIDTH_MM) * 0.5 or absf(pos.z) + half.y + FIT_GAP > mm(BOARD_DEPTH_MM) * 0.5:
        return false
    var center := Vector2(pos.x, pos.z)
    for hole in [Vector2(-3.5, -2), Vector2(3.5, -2), Vector2(-3.5, 2), Vector2(3.5, 2)]:
        var nearest := Vector2(clampf(hole.x, pos.x-half.x, pos.x+half.x), clampf(hole.y, pos.z-half.y, pos.z+half.y))
        if nearest.distance_to(hole) < 0.30 + FIT_GAP:
            return false
    for body in placed_components:
        if not is_instance_valid(body) or body == ignored:
            continue
        var other := _rotated_half(str(body.get_meta("component_id")), body.rotation.y)
        var delta := center - Vector2(body.position.x, body.position.z)
        if absf(delta.x) < half.x + other.x + FIT_GAP and absf(delta.y) < half.y + other.y + FIT_GAP:
            return false
    return true

func _nearest_fit(id: String, pos: Vector3, ignored: StaticBody3D = null, angle: float = 0.0) -> Vector3:
    if pos == Vector3.INF:
        return pos
    if _fit_at(id, pos, ignored, angle):
        return pos
    var best := Vector3.INF
    var best_distance := INF
    # Local assistance only: never teleport a component across the board.
    for x in range(-3, 4):
        for z in range(-3, 4):
            var candidate := pos + Vector3(x * BUILD_GRID, 0, z * BUILD_GRID)
            var distance := candidate.distance_squared_to(pos)
            if distance < best_distance and _fit_at(id, candidate, ignored, angle):
                best = candidate
                best_distance = distance
    return best

func _place_component_at_world(pos: Vector3) -> StaticBody3D:
    var id := selected_component_id
    if not history_replaying and not _fit_at(id, pos):
        return null
    _commit_history_snapshot()
    var body := super._place_component_at_world(pos)
    if body != null:
        selected_component_id = id # repeat placement without reopening the catalog.
        active_component = body
        _commit_history_snapshot()
        _refresh_inventory()
        _hide_world_text()
        engineering_state = "UNKNOWN"
        connectivity_found = false
    return body

func _handle_world_click(screen_pos: Vector2) -> void:
    if not wire_mode and not selected_component_id.is_empty():
        var target := _nearest_fit(selected_component_id, _screen_to_board(screen_pos))
        if target != Vector3.INF:
            _place_component_at_world(target)
        return
    super._handle_world_click(screen_pos)

func _refresh_preview_id() -> void:
    var id := catalog_drag_component_id if catalog_dragging else selected_component_id
    if id == preview_id:
        return
    if placement_preview != null:
        placement_preview.free()
        placement_preview = null
    preview_id = id
    if id.is_empty():
        return
    placement_preview = Node3D.new()
    placement_preview.name = "PlacementPreview3D"
    var probe := StaticBody3D.new()
    _build_component_visual(probe, _component_by_id(id), "")
    for child in probe.get_children():
        probe.remove_child(child)
        placement_preview.add_child(child)
    probe.free()
    add_child(placement_preview)

func _update_preview(screen_pos: Vector2) -> void:
    _refresh_preview_id()
    if placement_preview == null:
        return
    var raw := _screen_to_board(screen_pos)
    preview_target = _nearest_fit(preview_id, raw)
    preview_valid = preview_target != Vector3.INF and int(inventory_stock.get(preview_id, 0)) > 0
    var shown := preview_target if preview_valid else raw
    var over_ui := get_viewport().gui_get_hovered_control() != null and not catalog_dragging
    placement_preview.visible = shown != Vector3.INF and not over_ui and not wire_mode
    if not placement_preview.visible:
        return
    placement_preview.position = shown
    var color := Color(0.0, 0.85, 1.0, 0.45) if preview_valid else Color(1.0, 0.18, 0.12, 0.45)
    var mat := _material(color, 0.0, 0.6)
    mat.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
    for child in placement_preview.get_children():
        if child is MeshInstance3D:
            child.material_override = mat

func _update_catalog_drag_visual(screen_pos: Vector2) -> void:
    _update_preview(screen_pos)
    catalog_drop_world = preview_target
    catalog_drop_valid = preview_valid
    if catalog_drag_ghost != null:
        catalog_drag_ghost.visible = false
    if catalog_drop_marker != null:
        catalog_drop_marker.visible = false

func _process(delta: float) -> void:
    super._process(delta)
    if smooth_hud == null:
        return
    _update_preview(get_viewport().get_mouse_position())
    _hide_world_text()
    if tool_buttons.has("undo"):
        tool_buttons.undo.disabled = history_index <= 0
        tool_buttons.redo.disabled = history_index >= edit_history.size() - 1
        tool_buttons.wire.modulate = Color(0.45, 0.9, 1.0) if wire_mode else Color.WHITE

func _unhandled_key_input(event: InputEvent) -> void:
    # Inherited controls used C for both clearing and toggling; H also toggled a
    # hidden hint. Consume them before the legacy input route can mutate the PCB.
    if event is InputEventKey and event.pressed and not event.echo:
        if event.keycode == KEY_C:
            _toggle_catalog()
            get_viewport().set_input_as_handled()
            return
        if event.keycode == KEY_H:
            _toggle_inspection_help()
            get_viewport().set_input_as_handled()
            return
    super._unhandled_key_input(event)

func _unhandled_input(event: InputEvent) -> void:
    if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_RIGHT:
        _cancel_build() # cancel first; avoid accidental deletion while building.
        get_viewport().set_input_as_handled()
        return
    if event is InputEventKey and event.pressed and event.keycode == KEY_ESCAPE:
        _cancel_build()
        get_viewport().set_input_as_handled()
        return
    if event is InputEventMouseMotion and dragging_component != null:
        var id := str(dragging_component.get_meta("component_id"))
        var target := _nearest_fit(id, _screen_to_board(event.position), dragging_component, dragging_component.rotation.y)
        if target != Vector3.INF:
            dragging_component.position = target
            _refresh_wires_for_component(dragging_component)
            _reset_player_test_visuals()
            engineering_state = "UNKNOWN"
            connectivity_found = false
        get_viewport().set_input_as_handled()
        return
    if event is InputEventKey and event.pressed and event.keycode == KEY_DELETE:
        _remove_at_screen(get_viewport().get_mouse_position())
        get_viewport().set_input_as_handled()
        return
    super._unhandled_input(event)

func rotate_active_component(degrees: float) -> void:
    if active_component == null or not is_instance_valid(active_component):
        return
    var id := str(active_component.get_meta("component_id"))
    var angle := active_component.rotation.y + deg_to_rad(degrees)
    if not _fit_at(id, active_component.position, active_component, angle):
        return
    _commit_history_snapshot()
    super.rotate_active_component(degrees)
    _commit_history_snapshot()

func _test_player_circuit() -> void:
    _reset_player_test_visuals()
    last_player_test_passed = false
    last_player_test_path_length = 0
    connectivity_found = not _find_connected_power_led_path().is_empty()
    engineering_state = "UNKNOWN"
    status_label.text = "Connectivity only; electrical validation UNKNOWN."
    tool_buttons.test.modulate = Color(0.7, 0.75, 0.85)
    _refresh_help()
    help_panel.visible = true # an explicit check requests the detailed result.

func _connect_parts(a: StaticBody3D, b: StaticBody3D) -> void:
    _commit_history_snapshot()
    super._connect_parts(a, b)
    _commit_history_snapshot()
    engineering_state = "UNKNOWN"
    connectivity_found = false

func _restore_board_snapshot(snapshot: Dictionary) -> void:
    super._restore_board_snapshot(snapshot)
    engineering_state = "UNKNOWN"
    connectivity_found = false
    _hide_world_text()

func _remove_at_screen(screen_pos: Vector2) -> void:
    _commit_history_snapshot()
    super._remove_at_screen(screen_pos)
    _commit_history_snapshot()
    engineering_state = "UNKNOWN"
    connectivity_found = false
