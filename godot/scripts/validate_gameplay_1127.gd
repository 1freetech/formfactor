extends SceneTree

var failures: Array[String] = []

func _initialize() -> void:
    call_deferred("_run_validation")

func _check(condition: bool, message: String) -> void:
    if not condition:
        failures.append(message)
        push_error("1.127 validation: %s" % message)

func _run_validation() -> void:
    var application_name := str(ProjectSettings.get_setting("application/config/name", ""))
    _check(application_name == "FormFactor 1.127", "application version must be FormFactor 1.127")

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

    _check(lab is Node3D, "main playable scene must remain Node3D")

    for method_name in [
        "debug_catalog_place",
        "debug_force_history_commit",
        "debug_undo_board_edit",
        "debug_current_visual_present",
        "debug_connect_refs",
        "debug_move_refdes",
        "debug_wire_visuals_current",
        "debug_schematic_connection_count",
        "debug_primary_board_is_3d",
        "debug_schematic_is_2d_mirror"
    ]:
        _check(lab.has_method(method_name), "missing validation method: %s" % method_name)

    if not failures.is_empty():
        lab.queue_free()
        quit(1)
        return

    # Physical PCB stays 3D; schematic stays a separate 2D mirror.
    _check(bool(lab.debug_primary_board_is_3d()), "physical PCB must be a perspective 3D MeshInstance3D workspace")
    _check(bool(lab.debug_schematic_is_2d_mirror()), "schematic must be a separate 2D HUD mirror")

    var pcb := lab.get_node_or_null("PCB_Substrate")
    _check(pcb is MeshInstance3D, "PCB_Substrate must remain 3D geometry")

    var mirror := lab.get_node_or_null("HUD/SchematicMirrorPanel")
    _check(mirror is Control, "schematic mirror must remain a 2D Control panel")

    # Preserve the 1.126 editor fixes while advancing the version.
    _check(bool(lab.debug_catalog_place("resistor", Vector3(-2.0, 0.24, 0.0))), "must place resistor for history test")
    _check(bool(lab.debug_force_history_commit()), "resistor placement must commit to history")
    lab.call("_clear_player_board")
    _check(bool(lab.debug_force_history_commit()), "clear-board edit must commit to history")
    _check(bool(lab.debug_undo_board_edit()), "undo must restore the resistor state")
    await process_frame
    await process_frame
    _check(bool(lab.debug_current_visual_present("R1", "AxialResistorBody3D")), "undo must restore current resistor geometry without floating text")

    lab.call("_clear_player_board")
    lab.call("select_component", "power")
    var power := lab.call("_place_component_at_world", Vector3(-1.5, 0.24, 0.0)) as StaticBody3D
    lab.call("select_component", "led")
    var led := lab.call("_place_component_at_world", Vector3(1.5, 0.24, 0.0)) as StaticBody3D
    _check(power != null and led != null, "wire test parts must place")
    _check(bool(lab.debug_connect_refs("PWR1", "D1")), "power and LED must connect")
    _check(int(lab.debug_wire_count()) == 1, "wire test must create exactly one wire")
    _check(bool(lab.debug_wire_visuals_current()), "new wire must use current height and thickness")

    _check(bool(lab.debug_move_refdes("D1", Vector3(1.5, 0.24, 0.5))), "connected LED must move to an open grid position")
    await process_frame
    _check(bool(lab.debug_wire_visuals_current()), "moved wire must keep the same height and thickness")

    lab.call("_refresh_schematic")
    await process_frame
    _check(int(lab.debug_schematic_connection_count()) == 1, "2D schematic mirror must receive the real 3D board connection")

    lab.queue_free()
    await process_frame

    if failures.is_empty():
        print("FormFactor 1.127 validation: PASS")
        quit(0)
    else:
        print("FormFactor 1.127 validation: FAIL (%d)" % failures.size())
        quit(1)
