extends "res://scripts/interactive_lab_1122.gd"

# 1.123 adapts the open-source FreeSO catalogue interaction pattern to Godot:
# catalogue tiles own pointer/hover state while the parent build lab owns placement.
const CATALOG_DRAG_THRESHOLD := 8.0

var catalog_drag_component_id := ""
var catalog_hover_component_id := ""
var catalog_drag_origin := Vector2.ZERO
var catalog_dragging := false
var catalog_drop_valid := false
var catalog_drop_world := Vector3.INF
var catalog_drag_ghost: Label
var catalog_drop_marker: MeshInstance3D
var catalog_drop_ok_material: StandardMaterial3D
var catalog_drop_blocked_material: StandardMaterial3D

func _ready() -> void:
    super._ready()
    _build_catalog_drop_marker()
    if step_label != null:
        step_label.text = "START // Empty PCB. Choose a component from the Build / Buy catalog."
    if status_label != null:
        status_label.text = "Blank board ready. Click a component to select it, or drag a catalog tile straight onto the PCB."
    if build_status != null:
        build_status.text = "BUILD / BUY: click to select • drag to place"

# The old material-lab tutorial pre-populated a power source, LED, and copper path.
# The construction game now begins from an actually empty PCB instead.
func _build_power_source() -> void:
    pass

func _build_led() -> void:
    pass

func _build_copper_path() -> void:
    pass

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label == null:
                continue
            if label.text.begins_with("FORMFACTOR 1.122"):
                label.text = "FORMFACTOR 1.123 // BLANK BOARD BUILD / BUY LAB"
            elif label.text.begins_with("COMPONENT INVENTORY"):
                label.text = "BUILD / BUY // COMPONENT CATALOG"
            elif label.text.begins_with("LEFT CLICK PARTS"):
                label.text = "CLICK TILE = SELECT • DRAG TILE = PLACE • DRAG PLACED PART = MOVE • WHEEL = ZOOM"
    for child in find_children("*", "Label3D", true, false):
        var label3d := child as Label3D
        if label3d != null and label3d.text.begins_with("1.122 //"):
            label3d.text = "1.123 // EMPTY PCB • PICK • DRAG • BUILD"

func _build_gameplay_hud() -> void:
    super._build_gameplay_hud()
    var hud := get_node_or_null("HUD") as CanvasLayer
    if hud == null:
        return

    var inventory_panel := hud.get_node_or_null("InventoryPanel") as ColorRect
    if inventory_panel != null:
        for child in inventory_panel.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("COMPONENT INVENTORY"):
                label.text = "BUILD / BUY // COMPONENT CATALOG"
        if selected_label != null:
            selected_label.text = "CLICK = select • DRAG = place directly on PCB"

    catalog_drag_ghost = Label.new()
    catalog_drag_ghost.name = "CatalogDragGhost"
    catalog_drag_ghost.visible = false
    catalog_drag_ghost.mouse_filter = Control.MOUSE_FILTER_IGNORE
    catalog_drag_ghost.z_index = 100
    catalog_drag_ghost.add_theme_color_override("font_color", NEON)
    catalog_drag_ghost.add_theme_color_override("font_outline_color", Color(0.0, 0.0, 0.0, 0.96))
    catalog_drag_ghost.add_theme_constant_override("outline_size", 6)
    catalog_drag_ghost.add_theme_font_size_override("font_size", 16)
    hud.add_child(catalog_drag_ghost)

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
        var haystack := (str(component.name) + " " + str(component.prefix) + " " + str(component.symbol) + " " + str(component.category)).to_lower()
        if not query.is_empty() and haystack.find(query) == -1:
            continue

        var component_id := str(component.id)
        var count := int(inventory_stock.get(component_id, 0))
        var button := Button.new()
        button.name = "Inventory_%s" % component_id
        button.text = "%s  %-22s x%-2d   %s" % [component.prefix, component.name, count, component.symbol]
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

func select_component(component_id: String) -> void:
    super.select_component(component_id)
    if selected_component_id == component_id:
        var component := _component_by_id(component_id)
        if not component.is_empty() and status_label != null:
            status_label.text = "Selected %s. Click the PCB to place it, or drag its catalog tile directly onto the board." % str(component.name)
        _refresh_inventory()

func _catalog_tile_gui_input(event: InputEvent, component_id: String) -> void:
    if event is not InputEventMouseButton:
        return
    var mouse_button := event as InputEventMouseButton
    if mouse_button.button_index != MOUSE_BUTTON_LEFT:
        return
    if mouse_button.pressed:
        if int(inventory_stock.get(component_id, 0)) <= 0:
            return
        catalog_drag_component_id = component_id
        catalog_drag_origin = get_viewport().get_mouse_position()
        catalog_dragging = false
        catalog_drop_valid = false
        catalog_drop_world = Vector3.INF

func _catalog_tile_hover(component_id: String) -> void:
    catalog_hover_component_id = component_id
    if catalog_dragging:
        return
    var component := _component_by_id(component_id)
    if component.is_empty() or build_status == null:
        return
    build_status.text = "CATALOG: %s // click to select • drag to place" % str(component.name)

func _catalog_tile_exit(component_id: String) -> void:
    if catalog_hover_component_id == component_id:
        catalog_hover_component_id = ""
    if not catalog_dragging and build_status != null:
        build_status.text = "BUILD / BUY: click to select • drag to place"

func _input(event: InputEvent) -> void:
    if catalog_drag_component_id.is_empty():
        return

    if event is InputEventMouseMotion:
        var motion := event as InputEventMouseMotion
        if not catalog_dragging and catalog_drag_origin.distance_to(motion.position) >= CATALOG_DRAG_THRESHOLD:
            _begin_catalog_drag(motion.position)
        if catalog_dragging:
            _update_catalog_drag_visual(motion.position)
            get_viewport().set_input_as_handled()
        return

    if event is InputEventMouseButton:
        var mouse_button := event as InputEventMouseButton
        if mouse_button.button_index == MOUSE_BUTTON_LEFT and not mouse_button.pressed:
            if catalog_dragging:
                _finish_catalog_drag(mouse_button.position)
                get_viewport().set_input_as_handled()
            else:
                _reset_catalog_drag_state()
        return

    if event is InputEventKey:
        var key := event as InputEventKey
        if key.pressed and not key.echo and key.keycode == KEY_ESCAPE:
            _cancel_catalog_drag("Drag cancelled. The PCB is unchanged.")
            get_viewport().set_input_as_handled()

func _begin_catalog_drag(screen_pos: Vector2) -> void:
    catalog_dragging = true
    var component := _component_by_id(catalog_drag_component_id)
    if catalog_drag_ghost != null:
        catalog_drag_ghost.text = "▣ %s  x%d" % [str(component.get("name", catalog_drag_component_id)), int(inventory_stock.get(catalog_drag_component_id, 0))]
        catalog_drag_ghost.visible = true
    if status_label != null:
        status_label.text = "Dragging %s. Release over an open PCB grid position to place it." % str(component.get("name", catalog_drag_component_id))
    _update_catalog_drag_visual(screen_pos)

func _update_catalog_drag_visual(screen_pos: Vector2) -> void:
    if catalog_drag_ghost != null:
        catalog_drag_ghost.position = screen_pos + Vector2(18.0, 16.0)

    catalog_drop_world = _screen_to_board(screen_pos)
    catalog_drop_valid = false
    if catalog_drop_marker == null:
        return
    if catalog_drop_world == Vector3.INF:
        catalog_drop_marker.visible = false
        return

    catalog_drop_marker.visible = true
    catalog_drop_marker.position = catalog_drop_world + Vector3(0.0, 0.025, 0.0)
    var blocker := _component_near_position(catalog_drop_world)
    catalog_drop_valid = blocker == null
    catalog_drop_marker.material_override = catalog_drop_ok_material if catalog_drop_valid else catalog_drop_blocked_material
    if build_status != null:
        if catalog_drop_valid:
            build_status.text = "DROP READY: grid %.1f, %.1f" % [catalog_drop_world.x, catalog_drop_world.z]
        else:
            build_status.text = "DROP BLOCKED: another component already uses that PCB area"

func _finish_catalog_drag(screen_pos: Vector2) -> void:
    _update_catalog_drag_visual(screen_pos)
    var component_id := catalog_drag_component_id
    var placed := false
    if catalog_drop_valid and catalog_drop_world != Vector3.INF and int(inventory_stock.get(component_id, 0)) > 0:
        selected_component_id = component_id
        wire_mode = false
        wire_start = null
        _show_lexicon(component_id)
        var body := _place_component_at_world(catalog_drop_world)
        placed = body != null

    if not placed:
        selected_component_id = ""
        _update_selected_label()
        if status_label != null:
            status_label.text = "Nothing placed. Drag a component tile onto an open spot inside the green PCB."
    _reset_catalog_drag_state()

func _cancel_catalog_drag(message: String) -> void:
    if status_label != null:
        status_label.text = message
    _reset_catalog_drag_state()

func _reset_catalog_drag_state() -> void:
    catalog_drag_component_id = ""
    catalog_dragging = false
    catalog_drop_valid = false
    catalog_drop_world = Vector3.INF
    if catalog_drag_ghost != null:
        catalog_drag_ghost.visible = false
    if catalog_drop_marker != null:
        catalog_drop_marker.visible = false
    if build_status != null:
        build_status.text = "BUILD / BUY: click to select • drag to place"

func _build_catalog_drop_marker() -> void:
    catalog_drop_marker = MeshInstance3D.new()
    catalog_drop_marker.name = "CatalogDropMarker3D"
    var mesh := CylinderMesh.new()
    mesh.top_radius = 0.43
    mesh.bottom_radius = 0.43
    mesh.height = 0.018
    mesh.radial_segments = 36
    catalog_drop_marker.mesh = mesh

    catalog_drop_ok_material = StandardMaterial3D.new()
    catalog_drop_ok_material.albedo_color = Color(0.22, 1.0, 0.08, 0.48)
    catalog_drop_ok_material.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
    catalog_drop_ok_material.emission_enabled = true
    catalog_drop_ok_material.emission = NEON
    catalog_drop_ok_material.emission_energy_multiplier = 2.5
    catalog_drop_ok_material.roughness = 0.24

    catalog_drop_blocked_material = StandardMaterial3D.new()
    catalog_drop_blocked_material.albedo_color = Color(1.0, 0.16, 0.08, 0.52)
    catalog_drop_blocked_material.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
    catalog_drop_blocked_material.emission_enabled = true
    catalog_drop_blocked_material.emission = Color(1.0, 0.12, 0.05, 1.0)
    catalog_drop_blocked_material.emission_energy_multiplier = 2.2
    catalog_drop_blocked_material.roughness = 0.24

    catalog_drop_marker.material_override = catalog_drop_ok_material
    catalog_drop_marker.visible = false
    add_child(catalog_drop_marker)

func _handle_world_click(screen_pos: Vector2) -> void:
    if wire_mode or not selected_component_id.is_empty():
        super._handle_world_click(screen_pos)
        return
    var collider := _ray_collider(screen_pos)
    if collider != null and bool(collider.get_meta("placed_component", false)):
        super._handle_world_click(screen_pos)
        return
    if status_label != null:
        status_label.text = "Empty PCB spot. Choose a catalog component first, or drag one here from Build / Buy."

func debug_start_board_empty() -> bool:
    return placed_components.is_empty() \
        and get_node_or_null("PowerBody3D") == null \
        and get_node_or_null("LedBody3D") == null \
        and get_node_or_null("CopperTrace_A") == null \
        and get_node_or_null("CopperTrace_B") == null \
        and get_node_or_null("CopperTrace_Center") == null \
        and get_node_or_null("Wire3D") == null

func debug_catalog_button_count() -> int:
    if inventory_list == null:
        return 0
    var count := 0
    for child in inventory_list.get_children():
        if child is Button and str(child.name).begins_with("Inventory_"):
            count += 1
    return count

func debug_catalog_drag_enabled() -> bool:
    return catalog_drag_ghost != null and catalog_drop_marker != null and CATALOG_DRAG_THRESHOLD > 0.0

func debug_catalog_place(component_id: String, world_pos: Vector3) -> bool:
    if int(inventory_stock.get(component_id, 0)) <= 0:
        return false
    selected_component_id = component_id
    var body := _place_component_at_world(world_pos)
    return body != null
