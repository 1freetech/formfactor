extends SceneTree

func _initialize() -> void:
    call_deferred("_run_validation")

func _fail(message: String) -> void:
    push_error("GODOT3D FAIL: " + message)
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

    for method_name in [
        "debug_start_board_empty",
        "select_component",
        "_place_component_at_world",
        "debug_connect_pin_refs",
        "_test_player_circuit",
        "debug_last_pin_precheck_passed",
        "debug_last_test_passed"
    ]:
        if not scene.has_method(method_name):
            _fail("missing required 3D gameplay method: %s" % method_name)
            return

    if not bool(scene.call("debug_start_board_empty")):
        _fail("the live 3D PCB must begin without the retired prebuilt demo circuit")
        return

    var meshes := scene.find_children("*", "MeshInstance3D", true, false)
    var cameras := scene.find_children("*", "Camera3D", true, false)
    var directional_lights := scene.find_children("*", "DirectionalLight3D", true, false)
    var omni_lights := scene.find_children("*", "OmniLight3D", true, false)

    if meshes.size() < 20:
        _fail("expected at least 20 real MeshInstance3D nodes, found %d" % meshes.size())
        return
    if cameras.is_empty():
        _fail("no Camera3D found")
        return
    if directional_lights.is_empty() or omni_lights.is_empty():
        _fail("real 3D lighting is missing")
        return

    var standard_material_count := 0
    var metallic_material_count := 0
    for mesh_node in meshes:
        var mesh_instance := mesh_node as MeshInstance3D
        if mesh_instance.material_override is StandardMaterial3D:
            standard_material_count += 1
            var mat := mesh_instance.material_override as StandardMaterial3D
            if mat.metallic > 0.5:
                metallic_material_count += 1

    if standard_material_count < 16:
        _fail("expected at least 16 StandardMaterial3D overrides, found %d" % standard_material_count)
        return
    if metallic_material_count < 4:
        _fail("expected metallic solder and board hardware materials")
        return

    scene.call("select_component", "power")
    var power := scene.call("_place_component_at_world", Vector3(-1.5, 0.24, 0.6)) as StaticBody3D
    scene.call("select_component", "resistor")
    var resistor := scene.call("_place_component_at_world", Vector3(0.0, 0.24, 0.6)) as StaticBody3D
    scene.call("select_component", "led")
    var led_body := scene.call("_place_component_at_world", Vector3(1.5, 0.24, 0.6)) as StaticBody3D
    await process_frame

    if power == null or resistor == null or led_body == null:
        _fail("player 3D component placement failed")
        return

    if not bool(scene.call("debug_connect_pin_refs", "PWR1", "POS", "R1", "1")):
        _fail("could not connect PWR1.POS to R1.1")
        return
    if not bool(scene.call("debug_connect_pin_refs", "R1", "2", "D1", "A")):
        _fail("could not connect R1.2 to D1.A")
        return
    if not bool(scene.call("debug_connect_pin_refs", "D1", "K", "PWR1", "NEG")):
        _fail("could not connect D1.K to PWR1.NEG")
        return
    scene.call("_test_player_circuit")
    await process_frame

    if not bool(scene.call("debug_last_pin_precheck_passed")) or not bool(scene.call("debug_last_test_passed")):
        _fail("player-built pin-level power-resistor-LED loop did not pass the implemented topology test")
        return

    var static_bodies := scene.find_children("*", "StaticBody3D", true, false)
    if static_bodies.size() < 3:
        _fail("expected at least three clickable player-placed 3D bodies")
        return

    var user_wire_count := 0
    for mesh_node in scene.find_children("*", "MeshInstance3D", true, false):
        var wire_mesh := mesh_node as MeshInstance3D
        if wire_mesh != null and (str(wire_mesh.name).begins_with("UserWire_") or str(wire_mesh.name).begins_with("UserPinWire_")):
            user_wire_count += 1
    if user_wire_count < 3:
        _fail("expected at least three real pin-level 3D user wire meshes, found %d" % user_wire_count)
        return

    var emissive_led_mesh_found := false
    for child in led_body.find_children("*", "MeshInstance3D", true, false):
        var led_mesh := child as MeshInstance3D
        if led_mesh != null and led_mesh.material_override is StandardMaterial3D:
            var led_mat := led_mesh.material_override as StandardMaterial3D
            if led_mat.emission_enabled:
                emissive_led_mesh_found = true
                break
    if not emissive_led_mesh_found:
        _fail("player-placed LED did not switch to an emissive StandardMaterial3D")
        return

    print("GODOT3D PASS: blank-board startup plus %d initial meshes, %d StandardMaterial3D materials, %d metallic materials, Camera3D, real lighting, %d player wire meshes, clickable bodies, and emissive LED feedback verified." % [meshes.size(), standard_material_count, metallic_material_count, user_wire_count])
    quit(0)
