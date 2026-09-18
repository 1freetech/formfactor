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
        "debug_power_network_snapshot_1127",
        "debug_last_test_passed",
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

    # Source-derived power-network pattern: verify topology propagation without
    # pretending that reachability is a voltage/current simulation.
    lab.call("_clear_player_board")
    lab.call("select_component", "power")
    var power := lab.call("_place_component_at_world", Vector3(-2.0, 0.24, 0.0)) as StaticBody3D
    lab.call("select_component", "resistor")
    var resistor := lab.call("_place_component_at_world", Vector3(0.0, 0.24, 0.0)) as StaticBody3D
    lab.call("select_component", "led")
    var led := lab.call("_place_component_at_world", Vector3(2.0, 0.24, 0.0)) as StaticBody3D
    _check(power != null and resistor != null and led != null, "power-network fixture parts must place")

    var network_open: Dictionary = lab.debug_power_network_snapshot_1127()
    _check(int(network_open.get("total_parts", -1)) == 3, "power network must count three placed parts")
    _check(int(network_open.get("source_reachable", -1)) == 1, "only the source is reachable before wires")
    _check(int(network_open.get("isolated_parts", -1)) == 2, "two parts must be isolated before wires")
    _check(int(network_open.get("powered_leds", -1)) == 0, "LED must not be source-reachable before wiring")

    lab.call("_connect_parts", power, resistor)
    var network_partial: Dictionary = lab.debug_power_network_snapshot_1127()
    _check(int(network_partial.get("source_reachable", -1)) == 2, "power propagation must reach the first wired receiver")
    _check(int(network_partial.get("isolated_parts", -1)) == 1, "unwired LED must remain isolated")

    lab.call("_connect_parts", resistor, led)
    var network_closed: Dictionary = lab.debug_power_network_snapshot_1127()
    _check(int(network_closed.get("links", -1)) == 2, "power network must count two valid wire links")
    _check(int(network_closed.get("source_reachable", -1)) == 3, "connected network must reach all three parts")
    _check(int(network_closed.get("isolated_parts", -1)) == 0, "connected network must have no isolated parts")
    _check(int(network_closed.get("powered_leds", -1)) == 1, "connected LED must be source-reachable")

    lab.call("_test_player_circuit")
    _check(bool(lab.debug_last_test_passed()), "connected topology must still pass the existing player circuit-path test")

    lab.queue_free()
    await process_frame

    if failures.is_empty():
        print("FormFactor 1.127 optimization validation: PASS")
        quit(0)
    else:
        print("FormFactor 1.127 optimization validation: FAIL (%d)" % failures.size())
        quit(1)
