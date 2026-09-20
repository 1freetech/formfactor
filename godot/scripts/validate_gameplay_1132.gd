extends SceneTree

func _init() -> void:
    var packed := load("res://scenes/main.tscn") as PackedScene
    if packed == null:
        push_error("1.132 validation: main 3D scene missing")
        quit(1)
        return
    var lab := packed.instantiate()
    root.add_child(lab)
    await process_frame
    if not lab.has_method("debug_1132_workbench_ready") or not lab.debug_1132_workbench_ready():
        push_error("1.132 validation: organized inspection workbench missing")
        quit(1)
        return
    lab._set_inspection_zoom(1.30)
    lab._set_inspection_mode(1)
    lab._set_inspection_zoom(0.80)
    lab._set_inspection_mode(0)
    if abs(lab.inspection_zoom - 1.30) > 0.001:
        push_error("1.132 validation: per-view zoom memory failed")
        quit(1)
        return
    lab.guides_visible = false
    if lab.board_guide != null:
        lab.board_guide.visible = false
    lab._reset_inspection_presentation()
    if lab.guides_visible or (lab.board_guide != null and lab.board_guide.visible):
        push_error("1.132 validation: reset overwrote guide preference")
        quit(1)
        return
    lab._toggle_inspection_help()
    if lab.view_hint.visible:
        push_error("1.132 validation: help declutter toggle failed")
        quit(1)
        return
    lab._toggle_inspection_toolbar()
    if lab.compact_toolbar.visible:
        push_error("1.132 validation: toolbar declutter toggle failed")
        quit(1)
        return
    if not lab.debug_1132_view_zoom_memory() or not lab.debug_1132_engineering_truth_untouched():
        push_error("1.132 validation: workbench/truth boundary failed")
        quit(1)
        return
    print("FormFactor 1.132 workbench validation: PASS")
    quit(0)
