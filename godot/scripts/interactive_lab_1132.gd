extends "res://scripts/interactive_lab_1131.gd"

# FormFactor 1.132
# Workbench presentation state only. This layer may organize camera/HUD controls,
# but it never creates or overrides electrical, thermal, timing, manufacturing,
# package, stackup, clearance, or simulation truth.

var inspection_hud_visible := true
var inspection_toolbar_visible := true
var view_zoom := [1.0, 1.0, 1.0, 1.0]

func _ready() -> void:
    super._ready()
    _organize_inspection_hud()
    _refresh_1132_presentation()

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.131"):
                label.text = "FORMFACTOR 1.132"
    var version_label := get_node_or_null("WorkbenchVersion3D") as Label3D
    if version_label != null:
        version_label.text = "1.132 // 3D PCB WORKBENCH"

func _organize_inspection_hud() -> void:
    # Move keyboard help to the right edge so the PCB and primary controls stay clear.
    if view_hint != null:
        view_hint.position = Vector2(1120, 20)
        view_hint.text = "I VIEW   G GUIDES   +/- ZOOM   0 RESET   H HELP   C CONTROLS"
        view_hint.add_theme_font_size_override("font_size", 8)
    # One status line is enough: remove the duplicate standalone zoom line.
    if zoom_badge != null:
        zoom_badge.visible = false
    if mode_badge != null:
        mode_badge.position = Vector2(18, 82)

func _set_inspection_zoom(value: float) -> void:
    super._set_inspection_zoom(value)
    if inspection_mode >= 0 and inspection_mode < view_zoom.size():
        view_zoom[inspection_mode] = inspection_zoom
    _refresh_1132_presentation()

func _set_inspection_mode(mode: int) -> void:
    var previous_mode := inspection_mode
    if previous_mode >= 0 and previous_mode < view_zoom.size():
        view_zoom[previous_mode] = inspection_zoom
    super._set_inspection_mode(mode)
    inspection_zoom = view_zoom[inspection_mode]
    if camera != null:
        if camera.projection == Camera3D.PROJECTION_ORTHOGONAL:
            camera.size = 10.5 / inspection_zoom
        else:
            var base_fov := 46.0 if inspection_mode == 0 else 42.0
            camera.fov = clampf(base_fov / inspection_zoom, 30.0, 62.0)
    _refresh_1132_presentation()

func _reset_inspection_presentation() -> void:
    # Reset the camera without silently changing the player's guide preference.
    var keep_guides := guides_visible
    super._reset_inspection_presentation()
    guides_visible = keep_guides
    if board_guide != null:
        board_guide.visible = keep_guides
    for i in range(view_zoom.size()):
        view_zoom[i] = 1.0
    _refresh_1132_presentation()

func _toggle_inspection_help() -> void:
    inspection_hud_visible = not inspection_hud_visible
    if view_hint != null:
        view_hint.visible = inspection_hud_visible
    _set_short_status("Inspection help: %s" % ("on" if inspection_hud_visible else "off"))

func _toggle_inspection_toolbar() -> void:
    inspection_toolbar_visible = not inspection_toolbar_visible
    if compact_toolbar != null:
        compact_toolbar.visible = inspection_toolbar_visible
    _set_short_status("Inspection controls: %s" % ("open" if inspection_toolbar_visible else "hidden"))

func _refresh_1132_presentation() -> void:
    if mode_badge != null:
        var names := ["BOARD 3D", "TOP ORTHO", "FRONT 3D", "SIDE 3D"]
        mode_badge.text = "VIEW // %s   ZOOM // %d%%" % [names[inspection_mode], int(round(inspection_zoom * 100.0))]

func _unhandled_key_input(event: InputEvent) -> void:
    super._unhandled_key_input(event)
    if not event.pressed or event.echo:
        return
    if event.keycode == KEY_H:
        _toggle_inspection_help()
    elif event.keycode == KEY_C:
        _toggle_inspection_toolbar()

func debug_1132_workbench_ready() -> bool:
    return compact_toolbar != null and mode_badge != null and view_hint != null and zoom_badge != null

func debug_1132_view_zoom_memory() -> bool:
    return view_zoom.size() == 4 and inspection_zoom >= 0.70 and inspection_zoom <= 1.45

func debug_1132_engineering_truth_untouched() -> bool:
    return debug_1131_engineering_truth_untouched() and camera is Camera3D and board_guide != null
