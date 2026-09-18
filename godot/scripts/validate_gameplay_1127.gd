extends SceneTree

var failures: Array[String] = []

func _initialize() -> void:
    call_deferred("_run_validation")

func _check(condition: bool, message: String) -> void:
    if not condition:
        failures.append(message)
        push_error("1.127 optimization validation: %s" % message)

func _run_validation() -> void:
    var application_name := str(ProjectSettings.get_setting("application/config/name", ""))
    _check(application_name == "FormFactor 1.127", "application version must be FormFactor 1.127")

    var packed := load("res://scenes/main.tscn") as PackedScene
    _check(packed != null, "main scene must load through the stable current entrypoint")
    if packed == null:
        quit(1)
        return

    var lab := packed.instantiate()
    root.add_child(lab)
    await process_frame
    await process_frame
    await process_frame

    for method_name in [
        "debug_1127_visuals_ready",
        "debug_history_limit_1127",
        "debug_set_active_refdes_1127",
        "debug_duplicate_active_1127",
        "debug_delete_active_1127",
        "debug_placed_count",
        "debug_find_component"
    ]:
        _check(lab.has_method(method_name), "missing 1.127 validation method: %s" % method_name)

    _check(bool(lab.debug_1127_visuals_ready()), "placement grid, board-depth layer, and rim light must exist")
    _check(int(lab.debug_history_limit_1127()) == 96, "edit history must be bounded to 96 snapshots")

    lab.call("_clear_player_board")
    lab.call("select_component", "resistor")
    var first := lab.call("_place_component_at_world", Vector3(0.0, 0.24, 0.0)) as StaticBody3D
    _check(first != null, "first resistor must place on an empty grid point")
    var after_first := int(lab.debug_placed_count())

    lab.call("select_component", "led")
    var blocked := lab.call("_place_component_at_world", Vector3(0.0, 0.24, 0.0)) as StaticBody3D
    _check(blocked == null, "normal click placement must reject an occupied grid point")
    _check(int(lab.debug_placed_count()) == after_first, "blocked placement must not change the board")

    _check(bool(lab.debug_set_active_refdes_1127("R1")), "R1 must become the active component")
    _check(bool(lab.debug_duplicate_active_1127()), "Ctrl+D behavior must duplicate the active component into open space")
    await process_frame
    _check(int(lab.debug_placed_count()) == after_first + 1, "duplicate must add exactly one component")
    _check(lab.debug_find_component("R2") != null, "duplicate must receive the next stable resistor reference")

    _check(bool(lab.debug_set_active_refdes_1127("R2")), "duplicated R2 must become active")
    _check(bool(lab.debug_delete_active_1127()), "Delete/Backspace behavior must remove the active component")
    await process_frame
    _check(int(lab.debug_placed_count()) == after_first, "delete must return the board to one component")
    _check(lab.debug_find_component("R2") == null, "deleted component must leave the active board state")

    lab.queue_free()
    await process_frame

    if failures.is_empty():
        print("FormFactor 1.127 optimization validation: PASS")
        quit(0)
    else:
        print("FormFactor 1.127 optimization validation: FAIL (%d)" % failures.size())
        quit(1)
