extends SceneTree

func _initialize() -> void:
    call_deferred("_run_validation")

func _fail(message: String) -> void:
    push_error("GODOT SMART GAMEPLAY FAIL: " + message)
    quit(1)

func _has_label_prefix(scene: Node, type_name: String, prefix: String) -> bool:
    for child in scene.find_children("*", type_name, true, false):
        if child is Label and str((child as Label).text).begins_with(prefix):
            return true
        if child is Label3D and str((child as Label3D).text).begins_with(prefix):
            return true
    return false

func _has_versioned_label3d(scene: Node) -> bool:
    for child in scene.find_children("*", "Label3D", true, false):
        if child is Label3D:
            var text := str((child as Label3D).text)
            if text.find(" //") > 0 and text.substr(0, text.find(" //")).begins_with("1."):
                return true
    return false

func _run_validation() -> void:
    var application_name := str(ProjectSettings.get_setting("application/config/name", ""))
    if not application_name.begins_with("FormFactor "):
        _fail("Godot application identity no longer identifies FormFactor")
        return

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
        "debug_workspace_zone",
        "debug_set_hover_by_refdes",
        "debug_focus_best_target",
        "debug_set_dragging_by_refdes",
        "debug_move_dragging_to",
        "debug_clear_dragging",
        "debug_track_history"
    ]
    for method_name in required_methods:
        if not scene.has_method(method_name):
            _fail("missing smart-interaction method: %s" % method_name)
            return

    if not _has_label_prefix(scene, "Label", "FORMFACTOR "):
        _fail("visible HUD no longer identifies FormFactor")
        return
    if not _has_versioned_label3d(scene):
        _fail("visible 3D workbench no longer exposes a FormFactor update label")
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
    if not bool(scene.call("debug_set_hover_by_refdes", "R1")):
        _fail("could not set the resistor as the visible hover target")
        return
    var resistor_position := scene.call("debug_component_position", "R1") as Vector3
    scene.call("debug_focus_best_target")
    var hover_focus_target := scene.call("debug_last_focus_target") as Vector3
    var expected_hover_focus := Vector3(resistor_position.x, 0.0, resistor_position.z)
    if not hover_focus_target.is_equal_approx(expected_hover_focus):
        _fail("F/FOCUS ignored the visibly hovered component and used the older active selection")
        return

    var history_before_drag := int(scene.call("debug_history_count"))
    if not bool(scene.call("debug_set_dragging_by_refdes", "PWR1")):
        _fail("could not start the drag-history regression")
        return
    var drag_target := redone_position + Vector3(0.0, 0.0, 0.65)
    if not bool(scene.call("debug_move_dragging_to", drag_target)):
        _fail("could not move the dragging component")
        return
    scene.call("debug_track_history", 1.0)
    if int(scene.call("debug_history_count")) != history_before_drag:
        _fail("undo history recorded an intermediate position while the component was still being dragged")
        return

    scene.call("debug_clear_dragging")
    scene.call("debug_track_history", 0.0)
    scene.call("debug_track_history", 0.20)
    if int(scene.call("debug_history_count")) != history_before_drag + 1:
        _fail("one completed drag should create exactly one new undo snapshot")
        return
    if not (scene.call("debug_component_position", "PWR1") as Vector3).is_equal_approx(drag_target):
        _fail("dragged component did not remain at its released position")
        return
    if not bool(scene.call("debug_undo_board_edit")):
        _fail("clean drag snapshot could not be undone")
        return
    await process_frame
    await process_frame
    if not (scene.call("debug_component_position", "PWR1") as Vector3).is_equal_approx(redone_position):
        _fail("one undo after one drag did not restore the pre-drag position")
        return

    print("GODOT SMART GAMEPLAY PASS: current FormFactor identity is visible, hover focus follows the bright smart target, drag movement creates one clean undo step, and smart-target plus board-history systems remain valid.")
    quit(0)
