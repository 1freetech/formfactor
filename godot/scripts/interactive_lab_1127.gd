extends "res://scripts/interactive_lab_1126.gd"

# FormFactor 1.127
# Presentation boundary:
# - the physical PCB and physical component packages remain the primary 3D workspace;
# - the schematic stays a separate 2D mirror inside the HUD;
# - neither presentation layer changes engineering truth.

func _ready() -> void:
    super._ready()
    _enforce_3d_primary_workspace()
    if status_label != null:
        status_label.text = "3D PCB ready. Build on the physical board; the side schematic is a 2D mirror only."
    if build_status != null:
        build_status.text = "3D BUILD / BUY"

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
        version_label.text = "1.127 // 3D PCB + 2D SCHEMATIC MIRROR"

func _enforce_3d_primary_workspace() -> void:
    var pcb := get_node_or_null("PCB_Substrate") as MeshInstance3D
    if pcb != null:
        pcb.visible = true
        var board_material := pcb.material_override as StandardMaterial3D
        if board_material != null:
            board_material.metallic = 0.03
            board_material.roughness = 0.38

    if camera != null:
        camera.projection = Camera3D.PROJECTION_PERSPECTIVE
        camera.fov = 46.0

    var schematic_panel := get_node_or_null("HUD/SchematicMirrorPanel") as Control
    if schematic_panel != null:
        schematic_panel.visible = true
        schematic_panel.tooltip_text = "2D schematic mirror of the 3D physical PCB. This panel does not replace the board."

    var key_light := get_node_or_null("KeyLight3D") as DirectionalLight3D
    if key_light != null:
        key_light.light_energy = 1.85
        key_light.shadow_enabled = true

    var neon_fill := get_node_or_null("NeonFill3D") as OmniLight3D
    if neon_fill != null:
        neon_fill.light_energy = 2.6
        neon_fill.shadow_enabled = true

func debug_primary_board_is_3d() -> bool:
    var pcb := get_node_or_null("PCB_Substrate")
    return pcb is MeshInstance3D and camera is Camera3D and camera.projection == Camera3D.PROJECTION_PERSPECTIVE

func debug_schematic_is_2d_mirror() -> bool:
    var panel := get_node_or_null("HUD/SchematicMirrorPanel")
    return panel is Control and schematic_canvas is Control and panel.visible
