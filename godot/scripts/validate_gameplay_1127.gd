extends SceneTree

var failures: Array[String] = []

func _initialize() -> void:
    call_deferred("_run_validation")

func _check(condition: bool, message: String) -> void:
    if not condition:
        failures.append(message)
        push_error("1.127 presentation validation: %s" % message)

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
    _check(lab.has_method("debug_primary_board_is_3d"), "missing 3D-board regression check")
    _check(lab.has_method("debug_schematic_is_2d_mirror"), "missing 2D-schematic regression check")

    if lab.has_method("debug_primary_board_is_3d"):
        _check(bool(lab.debug_primary_board_is_3d()), "physical PCB must be a perspective 3D MeshInstance3D workspace")
    if lab.has_method("debug_schematic_is_2d_mirror"):
        _check(bool(lab.debug_schematic_is_2d_mirror()), "schematic must stay a separate 2D HUD mirror")

    var pcb := lab.get_node_or_null("PCB_Substrate")
    _check(pcb is MeshInstance3D, "PCB_Substrate must remain 3D geometry")

    var mirror := lab.get_node_or_null("HUD/SchematicMirrorPanel")
    _check(mirror is Control, "schematic mirror must remain a 2D Control panel")

    lab.queue_free()
    await process_frame

    if failures.is_empty():
        print("FormFactor 1.127 presentation validation: PASS")
        quit(0)
    else:
        print("FormFactor 1.127 presentation validation: FAIL (%d)" % failures.size())
        quit(1)
