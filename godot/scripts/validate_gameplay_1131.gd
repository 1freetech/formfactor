extends SceneTree

func _init() -> void:
    var packed := load("res://scenes/main.tscn") as PackedScene
    if packed == null:
        push_error("1.131 validation: main 3D scene missing")
        quit(1)
        return
    var lab := packed.instantiate()
    root.add_child(lab)
    await process_frame
    if not lab.has_method("debug_1131_workbench_ready") or not lab.debug_1131_workbench_ready():
        push_error("1.131 validation: compact inspection workbench missing")
        quit(1)
        return
    if not lab.debug_1131_zoom_bounded():
        push_error("1.131 validation: inspection zoom bounds invalid")
        quit(1)
        return
    lab._set_inspection_zoom(9.0)
    if lab.inspection_zoom > 1.45:
        push_error("1.131 validation: upper zoom clamp failed")
        quit(1)
        return
    lab._set_inspection_zoom(-9.0)
    if lab.inspection_zoom < 0.70:
        push_error("1.131 validation: lower zoom clamp failed")
        quit(1)
        return
    lab._reset_inspection_presentation()
    if lab.inspection_mode != 0 or not lab.guides_visible:
        push_error("1.131 validation: presentation reset failed")
        quit(1)
        return
    if not lab.debug_1131_engineering_truth_untouched():
        push_error("1.131 validation: engineering-truth boundary failed")
        quit(1)
        return
    print("FormFactor 1.131 workbench validation: PASS")
    quit(0)
