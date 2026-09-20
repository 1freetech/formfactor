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
    # This is a historical regression gate running against the current derived
    # workbench. Later versions legitimately extend the 1.129 four-button bar,
    # so validate the original controls without requiring the child count to
    # remain frozen at exactly four.
    if lab.inspection_toolbar == null or lab.inspection_toolbar.get_child_count() < 4:
        push_error("1.129 validation: base inspection toolbar missing")
        quit(1)
        return
    var expected := ["BOARD", "TOP", "LAYERS", "SCHEM"]
    for i in range(expected.size()):
        var button := lab.inspection_toolbar.get_child(i) as Button
        if button == null or button.text != expected[i]:
            push_error("1.129 validation: base inspection control %d invalid" % i)
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
