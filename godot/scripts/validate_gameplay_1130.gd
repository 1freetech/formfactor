extends SceneTree

func _init() -> void:
    var packed := load("res://scenes/main.tscn") as PackedScene
    if packed == null:
        push_error("1.130 validation: main 3D scene missing")
        quit(1)
        return
    var lab := packed.instantiate()
    root.add_child(lab)
    await process_frame
    if not lab.has_method("debug_1130_inspection_ready") or not lab.debug_1130_inspection_ready():
        push_error("1.130 validation: expanded inspection controls missing")
        quit(1)
        return
    lab._top_board_view()
    if lab.camera.projection != Camera3D.PROJECTION_ORTHOGONAL or lab.inspection_mode != 1:
        push_error("1.130 validation: top orthographic inspection failed")
        quit(1)
        return
    lab._front_board_view()
    if lab.inspection_mode != 2:
        push_error("1.130 validation: front inspection failed")
        quit(1)
        return
    lab._side_board_view()
    if lab.inspection_mode != 3:
        push_error("1.130 validation: side inspection failed")
        quit(1)
        return
    if not lab.debug_1130_views_available() or not lab.debug_1130_engineering_truth_untouched():
        push_error("1.130 validation: view state or engineering-truth boundary failed")
        quit(1)
        return
    print("FormFactor 1.130 inspection validation: PASS")
    quit(0)
