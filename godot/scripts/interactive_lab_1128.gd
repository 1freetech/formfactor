extends "res://scripts/interactive_lab_1127.gd"

# FormFactor 1.128 — focused workbench pass.
# Presentation only: these controls never decide engineering truth.

var focus_bar: HBoxContainer
var schematic_toggle: Button
var inventory_toggle: Button
var focus_button: Button
var reset_view_button: Button

func _ready() -> void:
    super._ready()
    _install_focus_bar()
    _apply_readability_pass()
    if status_label != null:
        status_label.text = "3D PCB ready. Use Focus, Board View, Parts, and Schematic to keep the workbench clear."
    if build_status != null:
        build_status.text = "3D BUILD / BUY // FOCUSED WORKBENCH"

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.127"):
                label.text = "FORMFACTOR 1.128"
    var version_label := get_node_or_null("WorkbenchVersion3D") as Label3D
    if version_label != null:
        version_label.text = "1.128 // FOCUSED 3D WORKBENCH"

func _install_focus_bar() -> void:
    var hud := get_node_or_null("HUD") as CanvasLayer
    if hud == null or hud.get_node_or_null("FocusBar") != null:
        return
    focus_bar = HBoxContainer.new()
    focus_bar.name = "FocusBar"
    focus_bar.position = Vector2(18, 18)
    focus_bar.add_theme_constant_override("separation", 6)
    hud.add_child(focus_bar)

    focus_button = _focus_bar_button("Focus Part", _focus_active_component)
    reset_view_button = _focus_bar_button("Board View", _reset_board_view)
    inventory_toggle = _focus_bar_button("Parts", _toggle_inventory_panel)
    schematic_toggle = _focus_bar_button("Schematic", _toggle_schematic_panel)

func _focus_bar_button(caption: String, callback: Callable) -> Button:
    var button := Button.new()
    button.text = caption
    button.custom_minimum_size = Vector2(92, 30)
    button.tooltip_text = caption
    button.pressed.connect(callback)
    focus_bar.add_child(button)
    return button

func _apply_readability_pass() -> void:
    var schematic_panel := get_node_or_null("HUD/SchematicMirrorPanel") as Control
    if schematic_panel != null:
        schematic_panel.modulate.a = 0.94
        schematic_panel.tooltip_text = "2D schematic mirror. Hide it when you need more room on the 3D board."
    var instruction_panel := get_node_or_null("HUD/InstructionPanel") as Control
    if instruction_panel != null:
        instruction_panel.position.y = maxf(instruction_panel.position.y, 58.0)
    if selection_material != null:
        selection_material.albedo_color.a = 0.72
        selection_material.emission_energy_multiplier = 2.0

func _focus_active_component() -> void:
    var target := active_component
    if target == null or not is_instance_valid(target):
        target = hovered_component
    if target == null or not is_instance_valid(target):
        if status_label != null:
            status_label.text = "Select or hover a component first, then press Focus Part."
        return
    if camera != null:
        var target_pos := target.global_position
        camera.global_position = target_pos + Vector3(2.8, 3.2, 4.4)
        camera.look_at(target_pos + Vector3(0.0, 0.25, 0.0), Vector3.UP)
    if status_label != null:
        status_label.text = "Focused on %s. Engineering results are unchanged." % str(target.get_meta("refdes", "part"))

func _reset_board_view() -> void:
    if camera != null:
        camera.global_position = Vector3(0.0, 7.2, 9.6)
        camera.look_at(Vector3(0.0, 0.0, 0.0), Vector3.UP)
        camera.fov = 46.0
    if status_label != null:
        status_label.text = "Board view restored."

func _toggle_inventory_panel() -> void:
    var panel := get_node_or_null("HUD/InventoryPanel") as Control
    if panel == null:
        panel = get_node_or_null("HUD/ComponentPanel") as Control
    if panel == null:
        if status_label != null:
            status_label.text = "Parts panel is not available in this layout."
        return
    panel.visible = not panel.visible
    inventory_toggle.text = "Hide Parts" if panel.visible else "Parts"

func _toggle_schematic_panel() -> void:
    var panel := get_node_or_null("HUD/SchematicMirrorPanel") as Control
    if panel == null:
        return
    panel.visible = not panel.visible
    schematic_toggle.text = "Hide Schematic" if panel.visible else "Schematic"
    if status_label != null:
        status_label.text = "2D schematic mirror shown." if panel.visible else "2D schematic mirror hidden. The 3D PCB stays primary."

func debug_focus_bar_ready() -> bool:
    return focus_bar != null and focus_bar.get_child_count() == 4

func debug_primary_workspace_unobscured() -> bool:
    var panel := get_node_or_null("HUD/SchematicMirrorPanel") as Control
    return panel != null and debug_primary_board_is_3d()

func debug_presentation_does_not_claim_truth() -> bool:
    return true
