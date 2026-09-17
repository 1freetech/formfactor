extends SceneTree

func _initialize() -> void:
    call_deferred("_run_validation")

func _fail(message: String) -> void:
    push_error("GODOT 1.123 FAIL: " + message)
    quit(1)

func _run_validation() -> void:
    var packed := load("res://scenes/main.tscn") as PackedScene
    if packed == null:
        _fail("main.tscn did not load")
        return

    var scene := packed.instantiate()
    root.add_child(scene)
    await process_frame
    await process_frame

    for method_name in [
        "debug_start_board_empty",
        "debug_catalog_button_count",
        "debug_catalog_drag_enabled",
        "debug_catalog_place",
        "debug_placed_count",
        "debug_position_available"
    ]:
        if not scene.has_method(method_name):
            _fail("missing 1.123 method: %s" % method_name)
            return

    if not bool(scene.call("debug_start_board_empty")):
        _fail("the playable PCB did not start blank")
        return

    if int(scene.call("debug_catalog_button_count")) != 25:
        _fail("expected all 25 component families in the Build / Buy catalog")
        return

    if not bool(scene.call("debug_catalog_drag_enabled")):
        _fail("catalog drag ghost or PCB drop marker did not initialize")
        return

    if int(scene.call("debug_placed_count")) != 0:
        _fail("placed component list was not empty at startup")
        return

    var placement := Vector3(0.0, 0.24, 0.0)
    if not bool(scene.call("debug_position_available", placement)):
        _fail("center PCB position should be open on the blank board")
        return

    if not bool(scene.call("debug_catalog_place", "resistor", placement)):
        _fail("catalog placement of a resistor failed")
        return
    await process_frame

    if int(scene.call("debug_placed_count")) != 1:
        _fail("catalog placement did not add exactly one component")
        return

    if bool(scene.call("debug_position_available", placement)):
        _fail("occupied PCB position still reports available")
        return

    if bool(scene.call("debug_catalog_place", "led", placement)):
        _fail("catalog placement allowed an overlapping component")
        return

    if int(scene.call("debug_placed_count")) != 1:
        _fail("blocked overlap changed the board component count")
        return

    print("GODOT 1.123 PASS: blank PCB startup, 25-item Build / Buy catalog, click/drag infrastructure, snapped placement, occupancy blocking, and component count behavior verified.")
    quit(0)
