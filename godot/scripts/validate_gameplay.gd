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

    for method_name in ["select_component", "_place_component_at_world", "_connect_parts", "_pan_view", "debug_component_count", "debug_placed_count", "debug_wire_count", "debug_pan_position"]:
        if not scene.has_method(method_name):
            _fail("missing interactive gameplay method: %s" % method_name)
            return

    var component_count := int(scene.call("debug_component_count"))
    if component_count < 25:
        _fail("expected at least 25 inventory component families, found %d" % component_count)
        return

    scene.call("select_component", "resistor")
    var resistor := scene.call("_place_component_at_world", Vector3(-0.5, 0.24, 0.8)) as StaticBody3D
    await process_frame
    if resistor == null or not bool(resistor.get_meta("placed_component", false)):
        _fail("resistor placement did not create a real clickable StaticBody3D")
        return

    scene.call("select_component", "led")
    var led := scene.call("_place_component_at_world", Vector3(0.5, 0.24, 0.8)) as StaticBody3D
    await process_frame
    if led == null:
        _fail("LED placement failed")
        return

    if int(scene.call("debug_placed_count")) != 2:
        _fail("expected two player-placed components")
        return

    scene.call("_connect_parts", resistor, led)
    await process_frame
    if int(scene.call("debug_wire_count")) != 1:
        _fail("wire tool did not create a user wire")
        return

    var before_pan := scene.call("debug_pan_position") as Vector3
    scene.call("_pan_view", Vector2(1.0, 0.0))
    var after_pan := scene.call("debug_pan_position") as Vector3
    if is_equal_approx(before_pan.x, after_pan.x):
        _fail("right-pan control did not move the view")
        return

    var inventory_panel := scene.find_child("InventoryPanel", true, false)
    var schematic_panel := scene.find_child("SchematicMirrorPanel", true, false)
    var tools_panel := scene.find_child("BuildToolsPanel", true, false)
    if inventory_panel == null or schematic_panel == null or tools_panel == null:
        _fail("inventory, schematic mirror, or build controls panel is missing")
        return

    print("GODOT GAMEPLAY PASS: %d component families, placement, reference bodies, wiring, schematic UI, and intuitive pan controls verified." % component_count)
    quit(0)
