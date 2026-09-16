extends SceneTree

func _initialize() -> void:
    call_deferred("_run_validation")

func _fail(message: String) -> void:
    push_error("GODOT GAMEPLAY FAIL: " + message)
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
        "select_component", "_place_component_at_world", "_connect_parts", "_test_player_circuit",
        "_pan_view", "rotate_active_component", "debug_component_count", "debug_placed_count",
        "debug_wire_count", "debug_pan_position", "debug_position_available", "debug_last_test_passed",
        "debug_last_test_path_length", "debug_player_led_lit", "debug_lexicon_count",
        "debug_lexicon_complete", "debug_lexicon_panel_ready", "debug_select_first_placed",
        "debug_active_rotation_y"
    ]
    for method_name: String in required_methods:
        if not scene.has_method(method_name):
            _fail("missing interactive gameplay method: %s" % method_name)
            return

    var component_count: int = int(scene.call("debug_component_count"))
    if component_count != 25:
        _fail("expected exactly 25 inventory component families, found %d" % component_count)
        return

    var lexicon_count: int = int(scene.call("debug_lexicon_count"))
    if lexicon_count != component_count or not bool(scene.call("debug_lexicon_complete")):
        _fail("component lexicon does not cover every inventory family")
        return
    if not bool(scene.call("debug_lexicon_panel_ready")):
        _fail("component lexicon panel did not initialize")
        return

    scene.call("select_component", "power")
    var power := scene.call("_place_component_at_world", Vector3(-1.5, 0.24, 0.8)) as StaticBody3D
    await process_frame
    if power == null:
        _fail("power source placement failed")
        return

    scene.call("select_component", "resistor")
    var blocked := scene.call("_place_component_at_world", Vector3(-1.5, 0.24, 0.8)) as StaticBody3D
    await process_frame
    if blocked != null:
        _fail("overlapping placement was allowed on an occupied PCB position")
        return

    scene.call("select_component", "resistor")
    var resistor := scene.call("_place_component_at_world", Vector3(-0.5, 0.24, 0.8)) as StaticBody3D
    await process_frame
    if resistor == null:
        _fail("resistor placement failed after occupied-position rejection")
        return

    scene.call("select_component", "led")
    var led := scene.call("_place_component_at_world", Vector3(0.5, 0.24, 0.8)) as StaticBody3D
    await process_frame
    if led == null:
        _fail("LED placement failed")
        return

    if int(scene.call("debug_placed_count")) != 3:
        _fail("expected three player-placed components")
        return

    scene.call("_connect_parts", power, resistor)
    scene.call("_connect_parts", resistor, led)
    await process_frame
    if int(scene.call("debug_wire_count")) != 2:
        _fail("expected two player wires in the power-resistor-LED path")
        return

    scene.call("_test_player_circuit")
    await process_frame
    if not bool(scene.call("debug_last_test_passed")):
        _fail("connected power-to-LED circuit did not pass")
        return
    if int(scene.call("debug_last_test_path_length")) != 2:
        _fail("connected path did not report the expected two-wire path")
        return
    if not bool(scene.call("debug_player_led_lit")):
        _fail("player-placed LED did not visibly light after a passing circuit test")
        return

    if not bool(scene.call("debug_select_first_placed")):
        _fail("could not select a placed component for rotation")
        return
    var before_rotation: float = float(scene.call("debug_active_rotation_y"))
    scene.call("rotate_active_component", 90.0)
    var after_rotation: float = float(scene.call("debug_active_rotation_y"))
    if is_equal_approx(before_rotation, after_rotation):
        _fail("placed component rotation control did not change the part orientation")
        return

    var before_pan: Vector3 = scene.call("debug_pan_position") as Vector3
    scene.call("_pan_view", Vector2(1.0, 0.0))
    var after_pan: Vector3 = scene.call("debug_pan_position") as Vector3
    if is_equal_approx(before_pan.x, after_pan.x):
        _fail("right-pan control did not move the view")
        return

    var inventory_panel := scene.find_child("InventoryPanel", true, false)
    var schematic_panel := scene.find_child("SchematicMirrorPanel", true, false)
    var tools_panel := scene.find_child("BuildToolsPanel", true, false)
    var lexicon_panel := scene.find_child("ComponentLexiconPanel", true, false)
    if inventory_panel == null or schematic_panel == null or tools_panel == null or lexicon_panel == null:
        _fail("inventory, schematic mirror, build controls, or component lexicon panel is missing")
        return

    print("GODOT GAMEPLAY PASS: FormFactor 1.09 verified with 25-component menu + lexicon, collision-safe placement, connected-circuit LED feedback, placed-part rotation, schematic mirror, wiring, and camera controls.")
    quit(0)
