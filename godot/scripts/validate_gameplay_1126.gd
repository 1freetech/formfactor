extends SceneTree

var failures: Array[String] = []

func _initialize() -> void:
    call_deferred("_run_validation")

func _check(condition: bool, message: String) -> void:
    if not condition:
        failures.append(message)
        push_error("1.126 editor validation: %s" % message)

func _run_validation() -> void:
    var application_name := str(ProjectSettings.get_setting("application/config/name", ""))
    _check(application_name.begins_with("FormFactor 1."),
        "application identity must remain FormFactor with a three-decimal 1.x version")

    var packed := load("res://scenes/main.tscn") as PackedScene
    _check(packed != null, "main scene must load")
    if packed == null:
        quit(1)
        return

    var lab := packed.instantiate()
    root.add_child(lab)
    await process_frame
    await process_frame
    await process_frame

    for method_name in [
        "debug_catalog_place",
        "debug_force_history_commit",
        "debug_undo_board_edit",
        "debug_current_visual_present",
        "debug_connect_refs",
        "debug_move_refdes",
        "debug_wire_visuals_current",
        "debug_schematic_connection_count"
    ]:
        _check(lab.has_method(method_name), "missing editor validation method: %s" % method_name)

    if not failures.is_empty():
        lab.queue_free()
        quit(1)
        return

    # Fix 1: undo must restore the newest electronics-specific geometry.
    _check(bool(lab.debug_catalog_place("resistor", Vector3(-2.0, 0.24, 0.0))), "must place resistor for history test")
    _check(bool(lab.debug_force_history_commit()), "resistor placement must commit to history")
    lab.call("_clear_player_board")
    _check(bool(lab.debug_force_history_commit()), "clear-board edit must commit to history")
    _check(bool(lab.debug_undo_board_edit()), "undo must restore the resistor state")
    await process_frame
    await process_frame
    _check(bool(lab.debug_current_visual_present("R1", "AxialResistorBody3D")), "undo must restore the current axial resistor visual without floating component text")

    # Reset to a simple two-part board for wire and schematic checks.
    lab.call("_clear_player_board")
    lab.call("select_component", "power")
    var power := lab.call("_place_component_at_world", Vector3(-1.5, 0.24, 0.0)) as StaticBody3D
    lab.call("select_component", "led")
    var led := lab.call("_place_component_at_world", Vector3(1.5, 0.24, 0.0)) as StaticBody3D
    _check(power != null and led != null, "wire test parts must place")
    _check(bool(lab.debug_connect_refs("PWR1", "D1")), "power and LED must connect")
    _check(int(lab.debug_wire_count()) == 1, "wire test must create exactly one wire")
    _check(bool(lab.debug_wire_visuals_current()), "new wire must use current height and thickness")

    # Fix 2: moving a connected part must not change wire height/thickness.
    _check(bool(lab.debug_move_refdes("D1", Vector3(1.5, 0.24, 0.5))), "connected LED must move to an open grid position")
    await process_frame
    _check(bool(lab.debug_wire_visuals_current()), "moved wire must keep the same height and thickness as a newly created wire")

    # Fix 3: schematic mirror must receive the actual connection graph.
    lab.call("_refresh_schematic")
    await process_frame
    _check(int(lab.debug_schematic_connection_count()) == 1, "live schematic must mirror the real player-created connection")

    lab.queue_free()
    await process_frame

    if failures.is_empty():
        print("FormFactor 1.126 editor validation: PASS")
        quit(0)
    else:
        print("FormFactor 1.126 editor validation: FAIL (%d)" % failures.size())
        quit(1)
