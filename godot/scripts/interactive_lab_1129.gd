extends "res://scripts/interactive_lab_1128.gd"

# FormFactor 1.129
# Presentation-only inspection tools. They never create engineering evidence or
# change electrical, thermal, package, stackup, clearance, or manufacturing truth.

var inspection_toolbar: HBoxContainer
var layer_details_visible := true
var schematic_mirror_visible := true
var board_guide: Node3D
var default_camera_transform: Transform3D

func _ready() -> void:
    super._ready()
    if camera != null:
        default_camera_transform = camera.transform
    _fix_inspection_lighting()
    _build_board_guide()
    _build_inspection_toolbar()
    _shorten_workbench_status()
    if build_status != null:
        build_status.text = "3D BUILD // INSPECT"

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.128"):
                label.text = "FORMFACTOR 1.129"
    var version_label := get_node_or_null("WorkbenchVersion3D") as Label3D
    if version_label != null:
        version_label.text = "1.129 // 3D PCB INSPECTION"

func _fix_inspection_lighting() -> void:
    # 1.128's rim light was useful but too strong for long inspection sessions.
    if pbr_rim_light != null:
        pbr_rim_light.light_energy = 2.8
        pbr_rim_light.spot_angle = 54.0

func _build_board_guide() -> void:
    board_guide = Node3D.new()
    board_guide.name = "BoardInspectionGuide3D"
    add_child(board_guide)
    var guide_material := StandardMaterial3D.new()
    guide_material.albedo_color = Color(0.20, 0.95, 0.58, 0.72)
    guide_material.emission_enabled = true
    guide_material.emission = Color(0.05, 0.40, 0.18, 1.0)
    guide_material.emission_energy_multiplier = 0.45
    guide_material.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
    # Thin corner ticks improve board orientation without covering the canvas.
    for data in [
        ["GuideNW_X", Vector3(0.52, 0.012, 0.025), Vector3(-3.93, 0.184, -2.28)],
        ["GuideNW_Z", Vector3(0.025, 0.012, 0.52), Vector3(-4.18, 0.184, -2.03)],
        ["GuideSE_X", Vector3(0.52, 0.012, 0.025), Vector3(3.93, 0.184, 2.28)],
        ["GuideSE_Z", Vector3(0.025, 0.012, 0.52), Vector3(4.18, 0.184, 2.03)]
    ]:
        var marker := MeshInstance3D.new()
        marker.name = data[0]
        var mesh := BoxMesh.new()
        mesh.size = data[1]
        marker.mesh = mesh
        marker.position = data[2]
        marker.material_override = guide_material
        board_guide.add_child(marker)

func _build_inspection_toolbar() -> void:
    var hud := get_node_or_null("HUD") as CanvasLayer
    if hud == null:
        return
    inspection_toolbar = HBoxContainer.new()
    inspection_toolbar.name = "InspectionToolbar129"
    inspection_toolbar.position = Vector2(18, 18)
    inspection_toolbar.add_theme_constant_override("separation", 6)
    hud.add_child(inspection_toolbar)
    _add_inspection_button("BOARD", "Reset to the normal 3D board view.", _focus_board)
    _add_inspection_button("TOP", "Look straight down at the physical 3D PCB.", _top_board_view)
    _add_inspection_button("LAYERS", "Show or hide presentation-only board layer details.", _toggle_layer_details)
    _add_inspection_button("SCHEM", "Show or hide the 2D schematic mirror.", _toggle_schematic_mirror)

func _add_inspection_button(text: String, tip: String, action: Callable) -> void:
    var button := Button.new()
    button.text = text
    button.tooltip_text = tip
    button.custom_minimum_size = Vector2(70, 28)
    button.focus_mode = Control.FOCUS_NONE
    button.pressed.connect(action)
    inspection_toolbar.add_child(button)

func _focus_board() -> void:
    if camera == null:
        return
    camera.transform = default_camera_transform
    camera.projection = Camera3D.PROJECTION_PERSPECTIVE
    camera.fov = 46.0
    _set_short_status("Board view")

func _top_board_view() -> void:
    if camera == null:
        return
    camera.position = Vector3(0.0, 9.8, 0.01)
    camera.look_at(Vector3.ZERO, Vector3(0.0, 0.0, -1.0))
    camera.projection = Camera3D.PROJECTION_PERSPECTIVE
    camera.fov = 42.0
    _set_short_status("Top inspection view")

func _toggle_layer_details() -> void:
    layer_details_visible = not layer_details_visible
    if pbr_board_details != null:
        pbr_board_details.visible = layer_details_visible
    _set_short_status("Layer details: %s" % ("on" if layer_details_visible else "off"))

func _toggle_schematic_mirror() -> void:
    var panel := get_node_or_null("HUD/SchematicMirrorPanel") as Control
    if panel == null:
        return
    schematic_mirror_visible = not schematic_mirror_visible
    panel.visible = schematic_mirror_visible
    _set_short_status("2D schematic mirror: %s" % ("on" if schematic_mirror_visible else "off"))

func _shorten_workbench_status() -> void:
    _set_short_status("3D PCB ready // F board // T top // L layers // M schematic")

func _set_short_status(message: String) -> void:
    if status_label != null:
        status_label.text = message

func _unhandled_key_input(event: InputEvent) -> void:
    if not event.pressed or event.echo:
        return
    match event.keycode:
        KEY_F:
            _focus_board()
        KEY_T:
            _top_board_view()
        KEY_L:
            _toggle_layer_details()
        KEY_M:
            _toggle_schematic_mirror()

func debug_inspection_toolbar_ready() -> bool:
    return inspection_toolbar != null and inspection_toolbar.get_child_count() == 4

func debug_engineering_truth_untouched() -> bool:
    # This script exposes presentation controls only; no solver/pass state exists here.
    return pbr_board_details != null and camera is Camera3D and get_node_or_null("PCB_Substrate") is MeshInstance3D

func debug_board_guide_is_sparse() -> bool:
    return board_guide != null and board_guide.get_child_count() == 4
