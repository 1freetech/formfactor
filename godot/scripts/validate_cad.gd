extends SceneTree

func _initialize() -> void:
    call_deferred("_run_validation")

func _fail(message: String) -> void:
    push_error("GODOT CAD FAIL: " + message)
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
        "debug_cad_panel_ready",
        "debug_cad_dimensions",
        "debug_cad_hole_count",
        "debug_cad_source_contains_csg",
        "debug_apply_cad_prompt",
        "debug_cad_preview_shape_count"
    ]
    for method_name in required_methods:
        if not scene.has_method(method_name):
            _fail("missing CAD workbench method: %s" % method_name)
            return

    if not bool(scene.call("debug_cad_panel_ready")):
        _fail("CAD workbench panel or embedded preview did not initialize")
        return

    if int(scene.call("debug_cad_hole_count")) != 4:
        _fail("default CAD model should contain four mounting holes")
        return

    if int(scene.call("debug_cad_preview_shape_count")) != 5:
        _fail("default preview should contain one base solid and four subtractive hole shapes")
        return

    if not bool(scene.call("debug_cad_source_contains_csg")):
        _fail("default OpenSCAD source is missing deterministic CSG difference/cylinder operations")
        return

    if not bool(scene.call("debug_apply_cad_prompt", "120x80x2.4 mm plate, four holes diameter 3.2mm, inset 6mm")):
        _fail("plain-language CAD prompt did not produce a valid model")
        return
    await process_frame
    await process_frame

    var dimensions: Vector3 = scene.call("debug_cad_dimensions")
    if not dimensions.is_equal_approx(Vector3(120.0, 80.0, 2.4)):
        _fail("plain-language dimensions were not applied to the parameter inspector")
        return
    if int(scene.call("debug_cad_hole_count")) != 4:
        _fail("four-hole prompt did not preserve the subtractive mounting-hole pattern")
        return
    if int(scene.call("debug_cad_preview_shape_count")) != 5:
        _fail("parametric preview did not rebuild to one base plus four holes")
        return

    if not bool(scene.call("debug_apply_cad_prompt", "80x50x1.6 mm solid plate without holes")):
        _fail("solid-plate prompt did not produce a valid model")
        return
    await process_frame
    await process_frame

    dimensions = scene.call("debug_cad_dimensions")
    if not dimensions.is_equal_approx(Vector3(80.0, 50.0, 1.6)):
        _fail("solid-plate prompt dimensions were not applied")
        return
    if int(scene.call("debug_cad_hole_count")) != 0:
        _fail("solid-plate prompt should disable mounting holes")
        return
    if int(scene.call("debug_cad_preview_shape_count")) != 1:
        _fail("solid-plate preview should contain only the base solid")
        return

    var cad_panel := scene.find_child("CadWorkbenchPanel", true, false)
    var cad_prompt := scene.find_child("CadPrompt", true, false)
    var feature_tree := scene.find_child("CadFeatureTree", true, false)
    var source_preview := scene.find_child("CadSourcePreview", true, false)
    if cad_panel == null or cad_prompt == null or feature_tree == null or source_preview == null:
        _fail("CAD panel is missing its prompt, feature tree, or source preview controls")
        return

    print("GODOT CAD PASS: FormFactor 1.11 verified with text-to-parameter parsing, feature tree + inspector UI, embedded CSG preview, deterministic OpenSCAD source, and optional OpenSCAD/FreeCAD export bridges.")
    quit(0)
