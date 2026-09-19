extends "res://scripts/interactive_lab_1129.gd"

# FormFactor 1.130
# Presentation-only inspection workflow. Camera, guides, lighting, and HUD state
# never create or override electrical, thermal, timing, package, stackup,
# clearance, manufacturing, or simulation truth.

var inspection_mode := 0
var guides_visible := true
var compact_toolbar: HBoxContainer
var mode_badge: Label

func _ready() -> void:
    super._ready()
    _upgrade_inspection_toolbar()
    _build_mode_badge()
    _set_inspection_mode(0)

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.129"):
                label.text = "FORMFACTOR 1.130"
    var version_label := get_node_or_null("WorkbenchVersion3D") as Label3D
    if version_label != null:
        version_label.text = "1.130 // 3D PCB INSPECTION"

func _upgrade_inspection_toolbar() -> void:
    if inspection_toolbar == null:
        return
    # Keep the controls away from the title/status area and make the bar denser.
    inspection_toolbar.position = Vector2(18, 54)
    for child in inspection_toolbar.get_children():
        if child is Button:
            child.custom_minimum_size = Vector2(62, 26)
    _add_inspection_button("FRONT", "Inspect the physical PCB from the front edge.", _front_board_view)
    _add_inspection_button("SIDE", "Inspect the physical PCB from the side edge.", _side_board_view)
    _add_inspection_button("GUIDES", "Show or hide presentation-only orientation guides.", _toggle_board_guides)
    compact_toolbar = inspection_toolbar

func _build_mode_badge() -> void:
    var hud := get_node_or_null("HUD") as CanvasLayer
    if hud == null:
        return
    mode_badge = Label.new()
    mode_badge.name = "InspectionModeBadge130"
    mode_badge.position = Vector2(18, 86)
    mode_badge.add_theme_font_size_override("font_size", 11)
    mode_badge.modulate = Color(0.68, 0.92, 0.78, 0.88)
    hud.add_child(mode_badge)

func _focus_board() -> void:
    if camera == null:
        return
    camera.transform = default_camera_transform
    camera.projection = Camera3D.PROJECTION_PERSPECTIVE
    camera.fov = 46.0
    inspection_mode = 0
    _refresh_mode_badge()
    _set_short_status("Board view // I cycles inspection views")

func _top_board_view() -> void:
    if camera == null:
        return
    camera.position = Vector3(0.0, 9.8, 0.01)
    camera.look_at(Vector3.ZERO, Vector3(0.0, 0.0, -1.0))
    # Orthographic removes perspective distortion for placement inspection.
    camera.projection = Camera3D.PROJECTION_ORTHOGONAL
    camera.size = 10.5
    inspection_mode = 1
    _refresh_mode_badge()
    _set_short_status("Top inspection // orthographic presentation view")

func _front_board_view() -> void:
    if camera == null:
        return
    camera.position = Vector3(0.0, 2.7, 9.2)
    camera.look_at(Vector3.ZERO, Vector3.UP)
    camera.projection = Camera3D.PROJECTION_PERSPECTIVE
    camera.fov = 42.0
    inspection_mode = 2
    _refresh_mode_badge()
    _set_short_status("Front inspection view")

func _side_board_view() -> void:
    if camera == null:
        return
    camera.position = Vector3(9.2, 2.7, 0.0)
    camera.look_at(Vector3.ZERO, Vector3.UP)
    camera.projection = Camera3D.PROJECTION_PERSPECTIVE
    camera.fov = 42.0
    inspection_mode = 3
    _refresh_mode_badge()
    _set_short_status("Side inspection view")

func _toggle_board_guides() -> void:
    guides_visible = not guides_visible
    if board_guide != null:
        board_guide.visible = guides_visible
    _set_short_status("Orientation guides: %s" % ("on" if guides_visible else "off"))

func _cycle_inspection_view() -> void:
    _set_inspection_mode((inspection_mode + 1) % 4)

func _set_inspection_mode(mode: int) -> void:
    match mode:
        1:
            _top_board_view()
        2:
            _front_board_view()
        3:
            _side_board_view()
        _:
            _focus_board()

func _refresh_mode_badge() -> void:
    if mode_badge == null:
        return
    var names := ["BOARD 3D", "TOP ORTHO", "FRONT 3D", "SIDE 3D"]
    mode_badge.text = "VIEW // %s // PRESENTATION ONLY" % names[inspection_mode]

func _unhandled_key_input(event: InputEvent) -> void:
    # Preserve all inherited shortcuts first; 1.129 swallowed parent handling here.
    super._unhandled_key_input(event)
    if not event.pressed or event.echo:
        return
    if event.keycode == KEY_I:
        _cycle_inspection_view()
    elif event.keycode == KEY_G:
        _toggle_board_guides()

func debug_1130_inspection_ready() -> bool:
    return compact_toolbar != null and compact_toolbar.get_child_count() == 7 and mode_badge != null

func debug_1130_engineering_truth_untouched() -> bool:
    return debug_engineering_truth_untouched() and camera is Camera3D and board_guide != null

func debug_1130_views_available() -> bool:
    return inspection_mode >= 0 and inspection_mode <= 3 and guides_visible == board_guide.visible
