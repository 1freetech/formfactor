extends SceneTree

func _initialize() -> void:
    call_deferred("_run_validation")

func _fail(message: String) -> void:
    push_error("GAMEPLAY108 FAIL: " + message)
    quit(1)

func _run_validation() -> void:
    var packed := load("res://scenes/main.tscn") as PackedScene
    if packed == null:
        _fail("main.tscn did not load")
        return

    var scene := packed.instantiate()
    root.add_child(scene)
    await process_frame

    if not scene.has_method("debug_position_available") or not scene.has_method("debug_last_test_passed") or not scene.has_method("debug_connect_pin_refs"):
        _fail("1.08 gameplay hooks are missing")
        return

    scene.call("select_component", "power")
    var power := scene.call("_place_component_at_world", Vector3(-1.5, 0.24, 0.0)) as StaticBody3D
    scene.call("select_component", "led")
    var led := scene.call("_place_component_at_world", Vector3(1.5, 0.24, 0.0)) as StaticBody3D
    if power == null or led == null:
        _fail("power source or LED could not be placed")
        return

    scene.call("select_component", "resistor")
    var blocked := scene.call("_place_component_at_world", Vector3(-1.5, 0.24, 0.0)) as StaticBody3D
    if blocked != null:
        _fail("a second component was allowed to stack on an occupied grid position")
        return
    if int(scene.call("debug_placed_count")) != 2:
        _fail("blocked placement changed the player-board part count")
        return

    scene.call("select_component", "resistor")
    var resistor := scene.call("_place_component_at_world", Vector3(-0.5, 0.24, 1.0)) as StaticBody3D
    scene.call("select_component", "connector")
    var connector := scene.call("_place_component_at_world", Vector3(0.5, 0.24, 1.0)) as StaticBody3D
    if resistor == null or connector == null:
        _fail("stray-wire test parts could not be placed")
        return

    scene.call("debug_connect_pin_refs", "R1", "1", "J1", "1")
    scene.call("_test_player_circuit")
    if bool(scene.call("debug_last_test_passed")):
        _fail("TEST accepted a stray wire that does not connect power to the LED")
        return
    if bool(scene.call("debug_player_led_lit")):
        _fail("player LED lit without a continuous power path")
        return

    if not bool(scene.call("debug_connect_pin_refs", "PWR1", "POS", "D1", "A")):
        _fail("could not connect PWR1.POS to D1.A")
        return
    if not bool(scene.call("debug_connect_pin_refs", "D1", "K", "PWR1", "NEG")):
        _fail("could not connect D1.K to PWR1.NEG")
        return
    scene.call("_test_player_circuit")
    if not bool(scene.call("debug_last_test_passed")):
        _fail("closed pin-level power-to-LED loop did not pass TEST")
        return
    if int(scene.call("debug_last_test_path_length")) != 2:
        _fail("direct closed LED loop should contain exactly two external wires")
        return
    if not bool(scene.call("debug_player_led_lit")):
        _fail("the player-placed LED did not visibly light after a valid test")
        return

    print("GAMEPLAY108 PASS: occupied placement blocked, stray pin wire rejected, closed power-to-LED pin loop accepted, and player LED glow verified.")
    quit(0)
