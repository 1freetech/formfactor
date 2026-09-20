extends SceneTree

func _init() -> void:
    var packed := load("res://scenes/main.tscn") as PackedScene
    if packed == null:
        push_error("1.129 validation: main 3D scene missing")
        quit(1)
        return
    var lab := packed.instantiate()
    root.add_child(lab)
    await process_frame
    if not lab.has_method("debug_inspection_toolbar_ready") or not lab.debug_inspection_toolbar_ready():
        push_error("1.129 validation: inspection toolbar missing")
        quit(1)
        return
    if not lab.debug_board_guide_is_sparse():
        push_error("1.129 validation: sparse 3D board guide invalid")
        quit(1)
        return
    if not lab.debug_engineering_truth_untouched():
        push_error("1.129 validation: engineering-truth boundary failed")
        quit(1)
        return
    if lab.camera == null or not (lab.camera is Camera3D):
        push_error("1.129 validation: real 3D camera missing")
        quit(1)
        return
    print("FormFactor 1.129 inspection validation: PASS")
    quit(0)
