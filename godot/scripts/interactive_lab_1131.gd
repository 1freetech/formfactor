extends "res://scripts/interactive_lab_1130.gd"

# FormFactor 1.131
# Presentation-only workbench ergonomics. This layer may change camera and HUD
# state only. It must never create or override electrical, thermal, timing,
# manufacturing, package, stackup, clearance, or simulation truth.

var view_hint: Label
var zoom_badge: Label
var inspection_zoom := 1.0

func _ready() -> void:
    super._ready()
    _compact_inspection_controls()
    _build_inspection_help()
    _refresh_1131_presentation()

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.130"):
                label.text = "FORMFACTOR 1.131"
    var version_label := get_node_or_null("WorkbenchVersion3D") as Label3D
    if version_label != null:
        version_label.text = "1.131 // 3D PCB WORKBENCH"

func _compact_inspection_controls() -> void:
    if compact_toolbar == null:
        return
    # Seven controls no longer consume oversized horizontal space.
    compact_toolbar.position = Vector2(18, 52)
    for child in compact_toolbar.get_children():
        if child is Button:
            var button := child as Button
            button.custom_minimum_size = Vector2(54, 24)
            button.add_theme_font_size_override("font_size", 9)

func _build_inspection_help() -> void:
    var hud := get_node_or_null("HUD") as CanvasLayer
    if hud == null:
        return
    view_hint = Label.new()
    view_hint.name = "InspectionHint131"
    view_hint.position = Vector2(18, 108)
    view_hint.text = "I VIEW   G GUIDES   +/- ZOOM   0 RESET"
    view_hint.add_theme_font_size_override("font_size", 9)
    view_hint.modulate = Color(0.72, 0.78, 0.74, 0.72)
    hud.add_child(view_hint)

    zoom_badge = Label.new()
    zoom_badge.name = "InspectionZoom131"
    zoom_badge.position = Vector2(18, 124)
    zoom_badge.add_theme_font_size_override("font_size", 9)
    zoom_badge.modulate = Color(0.62, 0.86, 0.70, 0.76)
    hud.add_child(zoom_badge)

func _set_inspection_zoom(value: float) -> void:
    inspection_zoom = clampf(value, 0.70, 1.45)
    if camera == null:
        return
    if camera.projection == Camera3D.PROJECTION_ORTHOGONAL:
        camera.size = 10.5 / inspection_zoom
    else:
        camera.fov = clampf(46.0 / inspection_zoom, 30.0, 62.0)
    _refresh_1131_presentation()
    _set_short_status("Inspection zoom %d%% // presentation only" % int(round(inspection_zoom * 100.0)))

func _reset_inspection_presentation() -> void:
    inspection_zoom = 1.0
    guides_visible = true
    if board_guide != null:
        board_guide.visible = true
    _focus_board()
    _refresh_1131_presentation()
    _set_short_status("3D board presentation reset")

func _set_inspection_mode(mode: int) -> void:
    super._set_inspection_mode(mode)
    inspection_zoom = 1.0
    _refresh_1131_presentation()

func _refresh_1131_presentation() -> void:
    if zoom_badge != null:
        zoom_badge.text = "ZOOM // %d%%" % int(round(inspection_zoom * 100.0))
    if mode_badge != null:
        # Keep the mode badge concise; truth boundary is documented in code/tests.
        var names := ["BOARD 3D", "TOP ORTHO", "FRONT 3D", "SIDE 3D"]
        mode_badge.text = "VIEW // %s" % names[inspection_mode]

func _unhandled_key_input(event: InputEvent) -> void:
    super._unhandled_key_input(event)
    if not event.pressed or event.echo:
        return
    if event.keycode == KEY_EQUAL or event.keycode == KEY_KP_ADD:
        _set_inspection_zoom(inspection_zoom + 0.10)
    elif event.keycode == KEY_MINUS or event.keycode == KEY_KP_SUBTRACT:
        _set_inspection_zoom(inspection_zoom - 0.10)
    elif event.keycode == KEY_0 or event.keycode == KEY_KP_0:
        _reset_inspection_presentation()

func debug_1131_workbench_ready() -> bool:
    return compact_toolbar != null and view_hint != null and zoom_badge != null

func debug_1131_zoom_bounded() -> bool:
    return inspection_zoom >= 0.70 and inspection_zoom <= 1.45

func debug_1131_engineering_truth_untouched() -> bool:
    return debug_1130_engineering_truth_untouched() and camera is Camera3D and board_guide != null
