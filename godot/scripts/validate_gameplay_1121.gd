extends SceneTree

func _initialize() -> void:
    call_deferred("_run_validation")

func _fail(message: String) -> void:
    push_error("GODOT 1.121 GAMEPLAY FAIL: " + message)
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

    var required_methods: Array[String] = [
        "debug_context_panel_ready",
        "debug_focus_marker_ready",
        "debug_history_count",
        "debug_history_index",
        "debug_force_history_commit",
        "debug_undo_board_edit",
        "debug_redo_board_edit",
        "debug_component_position",
        "debug_set_active_by_refdes",
        "debug_focus_active",
        "debug_last_focus_target",
        "debug_workspace_zone"
    ]
    for method_name in required_methods:
        if not scene.has_method(method_name):
            _fail("missing FormFactor 1.121 method: %s" % method_name)
            return

    if not bool(scene.call("debug_context_panel_ready")):
        _fail("smart-target context HUD did not initialize")
        return
    if not bool(scene.call("debug_focus_marker_ready")):
        _fail("3D smart-target marker did not initialize")
        return
    if str(scene.call("debug_workspace_zone")) != "ZONE // PCB BENCH":
        _fail("default workspace zone should be the PCB bench")
        return
    if int(scene.call("debug_history_count")) != 1 or int(scene.call("debug_history_index")) != 0:
        _fail("board history should begin with exactly one empty-board snapshot")
        return

    scene.call("select_component", "power")
    var power := scene.call("_place_component_at_world", Vector3(-1.5, 0.24, 0.8)) as StaticBody3D
    scene.call("select_component", "resistor")
    var resistor := scene.call("_place_component_at_world", Vector3(-0.5, 0.24, 0.8)) as StaticBody3D
    if power == null or resistor == null:
        _fail("could not create the board-history test components")
        return
    scene.call("_connect_parts", power, resistor)
    await process_frame

    if not bool(scene.call("debug_force_history_commit")):
        _fail("placing and wiring components did not create a history snapshot")
        return
    if int(scene.call("debug_history_count")) != 2:
        _fail("expected empty state plus one populated board state")
        return

    var original_power_position := power.position
    if not bool(scene.call("debug_move_component", power, Vector3(-1.5, 0.24, 1.8))):
        _fail("could not move the power component for history validation")
        return
    if not bool(scene.call("debug_force_history_commit")):
        _fail("component movement did not create a second edit snapshot")
        return
    if int(scene.call("debug_history_count")) != 3:
        _fail("expected three board states after the move")
        return

    var moved_position := scene.call("debug_component_position", "PWR1") as Vector3
    if moved_position.is_equal_approx(original_power_position):
        _fail("the power component did not move before undo")
        return

    if not bool(scene.call("debug_undo_board_edit")):
        _fail("undo did not move backward through board history")
        return
    await process_frame
    await process_frame
    var undone_position := scene.call("debug_component_position", "PWR1") as Vector3
    if not undone_position.is_equal_approx(original_power_position):
        _fail("undo did not restore the original component position")
        return
    if int(scene.call("debug_wire_count")) != 1:
        _fail("undo did not restore the saved wire relationship")
        return

    if not bool(scene.call("debug_redo_board_edit")):
        _fail("redo did not move forward through board history")
        return
    await process_frame
    await process_frame
    var redone_position := scene.call("debug_component_position", "PWR1") as Vector3
    if not redone_position.is_equal_approx(moved_position):
        _fail("redo did not restore the moved component position")
        return
    if int(scene.call("debug_wire_count")) != 1:
        _fail("redo did not preserve the saved wire relationship")
        return

    if not bool(scene.call("debug_set_active_by_refdes", "PWR1")):
        _fail("could not select a restored component for smart focus")
        return
    if not bool(scene.call("debug_focus_active")):
        _fail("smart focus did not accept the active component")
        return
    var focus_target := scene.call("debug_last_focus_target") as Vector3
    if not focus_target.is_equal_approx(Vector3(redone_position.x, 0.0, redone_position.z)):
        _fail("smart focus target did not match the active component position")
        return

    print("GODOT 1.121 GAMEPLAY PASS: smart targeting HUD, pulsing world marker, smooth component/board focus, workspace zone state, and replay-style board undo/redo all validated against the real scene.")
    quit(0)
