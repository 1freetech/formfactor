extends SceneTree

func _initialize() -> void:
    call_deferred("_run_validation")

func _fail(message: String) -> void:
    push_error("GODOT ELECTRONICS 1.125 FAIL: " + message)
    quit(1)

func _run_validation() -> void:
    var application_name := str(ProjectSettings.get_setting("application/config/name", ""))
    if not application_name.begins_with("FormFactor 1."):
        _fail("application identity is not a FormFactor version")
        return

    var packed := load("res://scenes/main.tscn") as PackedScene
    if packed == null:
        _fail("main.tscn did not load")
        return

    var scene := packed.instantiate()
    root.add_child(scene)
    await process_frame
    await process_frame
    await process_frame

    for method_name in [
        "debug_board_is_visual_canvas",
        "debug_vector_schematic_ready",
        "debug_component_visual_has_text",
        "debug_component_count"
    ]:
        if not scene.has_method(method_name):
            _fail("missing 1.125 validation method: %s" % method_name)
            return

    if not bool(scene.call("debug_board_is_visual_canvas")):
        _fail("PCB still contains inherited text or generic starter pads")
        return
    if not bool(scene.call("debug_vector_schematic_ready")):
        _fail("vector schematic canvas did not initialize")
        return
    if int(scene.call("debug_component_count")) != 25:
        _fail("expected 25 current component families")
        return

    var version_marker_count := 0
    for child in scene.find_children("*", "Label3D", true, false):
        if child is Label3D and not child.is_queued_for_deletion():
            var label := child as Label3D
            if label.name == "WorkbenchVersion3D":
                version_marker_count += 1
    if version_marker_count != 1:
        _fail("expected one off-board workbench version marker")
        return

    var expected_nodes := {
        "resistor": "AxialResistorBody3D",
        "potentiometer": "PotBody3D",
        "ceramic_cap": "CeramicDisc3D",
        "electrolytic_cap": "ElectrolyticCan3D",
        "inductor": "InductorCore3D",
        "diode": "DiodeBody3D",
        "zener": "DiodeBody3D",
        "led": "LedLens3D",
        "npn": "TO92Body3D",
        "pnp": "TO92Body3D",
        "nmos": "TO220Body3D",
        "pmos": "TO220Body3D",
        "logic": "DIPPackage3D",
        "opamp": "DIPPackage3D",
        "comparator": "DIPPackage3D",
        "regulator": "RegulatorBody3D",
        "fuse": "FuseGlass3D",
        "switch": "SwitchBase3D",
        "relay": "RelayCase3D",
        "connector": "HeaderBody3D",
        "test_point": "TestPointPad3D",
        "sensor": "SensorCan3D",
        "buzzer": "BuzzerCase3D",
        "power": "PowerTerminalBlock3D",
        "ground": "GroundPad3D"
    }

    var ids: Array[String] = [
        "resistor", "potentiometer", "ceramic_cap", "electrolytic_cap",
        "inductor", "diode", "zener", "led", "npn", "pnp", "nmos", "pmos",
        "logic", "opamp", "comparator", "regulator", "fuse", "switch",
        "relay", "connector", "test_point", "sensor", "buzzer", "power",
        "ground"
    ]

    for i in range(ids.size()):
        var component_id := ids[i]
        scene.call("select_component", component_id)
        var col := i % 8
        var row := i / 8
        var position := Vector3(-3.5 + float(col), 0.24, -1.5 + float(row))
        var body := scene.call("_place_component_at_world", position) as StaticBody3D
        if body == null:
            _fail("could not place %s" % component_id)
            return
        if body.find_child(str(expected_nodes[component_id]), true, false) == null:
            _fail("%s did not build its electronics-specific 3D package" % component_id)
            return
        if bool(scene.call("debug_component_visual_has_text", body)):
            _fail("%s printed floating text onto the PCB/component" % component_id)
            return

    if int(scene.call("debug_placed_count")) != 25:
        _fail("not all 25 electronics component families were placed")
        return

    scene.call("_refresh_schematic")
    await process_frame
    if not bool(scene.call("debug_vector_schematic_ready")):
        _fail("vector schematic mirror disappeared after component placement")
        return

    print("GODOT ELECTRONICS 1.125 PASS: blank PCB canvas, off-board version marker, vector schematic mirror, and distinct text-free 3D package assemblies verified for all 25 component families.")
    quit(0)
