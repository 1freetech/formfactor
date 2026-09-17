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

    var meshes := scene.find_children("*", "MeshInstance3D", true, false)
    var cameras := scene.find_children("*", "Camera3D", true, false)
    var directional_lights := scene.find_children("*", "DirectionalLight3D", true, false)
    var omni_lights := scene.find_children("*", "OmniLight3D", true, false)
    var static_bodies := scene.find_children("*", "StaticBody3D", true, false)

    if meshes.size() < 20:
        _fail("expected at least 20 real MeshInstance3D nodes, found %d" % meshes.size())
        return
    if cameras.is_empty():
        _fail("no Camera3D found")
        return
    if directional_lights.is_empty() or omni_lights.is_empty():
        _fail("real 3D lighting is missing")
        return
    if static_bodies.size() < 2:
        _fail("clickable 3D power and LED bodies are missing")
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
        _fail("expected metallic copper/solder/terminal materials")
        return

    var led := scene.find_child("LedDomeMesh3D", true, false) as MeshInstance3D
    var wire := scene.find_child("Wire3D", true, false) as MeshInstance3D
    var pulse := scene.find_child("CurrentPulseMesh3D", true, false) as MeshInstance3D
    if led == null or wire == null or pulse == null:
        _fail("LED, wire, or current-pulse 3D mesh is missing")
        return
    if wire.visible or pulse.visible:
        _fail("circuit feedback must start off until the interaction completes")
        return

    if scene.has_method("_complete_circuit"):
        scene.call("_complete_circuit")
        await process_frame
    else:
        _fail("3D gameplay completion hook is missing")
        return

    var led_mat := led.material_override as StandardMaterial3D
    if led_mat == null or not led_mat.emission_enabled:
        _fail("LED does not switch to a real emissive StandardMaterial3D")
        return
    if not wire.visible or not pulse.visible:
        _fail("3D wire/current feedback did not activate")
        return

    print("GODOT3D PASS: %d meshes, %d StandardMaterial3D materials, %d metallic materials, Camera3D, DirectionalLight3D, OmniLight3D, clickable bodies, emissive LED, 3D wire, and animated current marker verified." % [meshes.size(), standard_material_count, metallic_material_count])
    quit(0)
