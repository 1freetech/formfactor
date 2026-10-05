extends SceneTree

var failures: Array[String] = []
func check(condition: bool, message: String) -> void:
    if not condition:
        failures.append(message)
        push_error(message)

func _initialize() -> void:
    call_deferred("run")

func run() -> void:
    var scene := load("res://scenes/main.tscn") as PackedScene
    var lab := scene.instantiate()
    root.add_child(lab)
    await process_frame
    await process_frame
    check(lab.get_script().resource_path.ends_with("1135.gd"), "live scene must use new UX")
    check(lab.placed_components.is_empty(), "startup must be a blank board")
    check(lab.catalog_grid.get_child_count() == 25, "all 25 component families must be available")
    check(not lab.help_panel.visible, "help must start closed")
    check(lab.camera is Camera3D, "construction must stay 3D")
    for node in lab.find_children("*", "Label3D", true, false):
        check(not node.visible, "no world text: " + str(node.name))
    for node in lab.get_node("HUD").find_children("*", "Label", true, false):
        check(not node.is_visible_in_tree(), "no gameplay text: " + str(node.name))
    for node in lab.smooth_hud.find_children("*", "Button", true, false):
        check(node.text.is_empty() and node.icon != null and node.tooltip_text.is_empty(), "buttons must be wordless icons")
    for component in lab.COMPONENTS:
        var id := str(component.id)
        check(lab._fit_at(id, Vector3(0, 0.24, 0)), "catalog part must fit in board center: " + id)
        check(not lab._fit_at(id, Vector3(3.9, 0.24, 0)), "reject board-edge overhang: " + id)
        check(not lab._fit_at(id, Vector3(3.5, 0.24, 2)), "reject mounting hole: " + id)
    lab.select_component("resistor")
    var r = lab._place_component_at_world(Vector3(0, 0.24, 0))
    check(r != null, "resistor placement failed")
    check(lab.selected_component_id == "resistor", "repeat placement lost selection")
    var stock: int = lab.inventory_stock.resistor
    check(lab._place_component_at_world(Vector3(0, 0.24, 0)) == null, "overlap accepted")
    check(lab.inventory_stock.resistor == stock, "rejected placement consumed stock")
    var fit: Vector3 = lab._nearest_fit("resistor", Vector3(0, 0.24, 0))
    check(fit != Vector3.INF and fit.distance_to(Vector3(0, 0.24, 0)) < 1.1, "local snap recovery failed")
    check(lab._fit_at("resistor", fit), "preview recovery does not fit")
    lab.catalog_drag_component_id = "resistor"
    lab.catalog_dragging = true
    var screen: Vector2 = lab.camera.unproject_position(Vector3(0, .24, 0))
    lab._update_catalog_drag_visual(screen)
    check(lab.catalog_drop_valid and lab.placement_preview.visible, "catalog drag must show a fitted 3D preview")
    check(lab.catalog_drag_ghost.visible == false, "drag must not display words")
    lab._reset_catalog_drag_state()
    var second = lab._place_component_at_world(fit)
    check(second != null, "repeat placement failed")
    lab._undo_board_edit()
    check(lab.placed_components.size() == 1, "undo should remove only the last placement")
    lab._redo_board_edit()
    check(lab.placed_components.size() == 2, "redo should restore placement")
    var before: String = lab._board_signature()
    var key := InputEventKey.new()
    key.keycode = KEY_C
    key.pressed = true
    lab._unhandled_key_input(key)
    check(lab._board_signature() == before, "catalog shortcut deleted the board")
    lab._cancel_build()
    lab._clear_player_board()
    lab.select_component("power")
    var p = lab._place_component_at_world(Vector3(-1.5, 0.24, 0))
    lab.select_component("led")
    var led = lab._place_component_at_world(Vector3(1.5, 0.24, 0))
    lab._connect_parts(p, led)
    lab._test_player_circuit()
    check(lab.connectivity_found, "component path should be recognized")
    check(not lab.last_player_test_passed and lab.engineering_state == "UNKNOWN", "unverified graph must not become an electrical PASS")
    lab.help_panel.visible = false
    lab._cancel_build()
    await process_frame
    check(lab.placement_preview == null, "cancel must clear placement preview")
    # Screenshot mode creates deterministic runtime evidence, never concept art.
    if "--capture" in OS.get_cmdline_user_args():
        lab.catalog_panel.visible = true
        lab._clear_player_board()
        var ids := ["power", "resistor", "led", "logic", "ceramic_cap", "connector"]
        var positions := [Vector3(-2, .24, -1), Vector3(0, .24, -1), Vector3(2, .24, -1), Vector3(-2, .24, 1), Vector3(0, .24, 1), Vector3(2, .24, 1)]
        for i in range(ids.size()):
            lab.select_component(ids[i])
            check(lab._place_component_at_world(positions[i]) != null, "screenshot fixture placement failed")
        lab._cancel_build()
        for i in range(8):
            await process_frame
        await RenderingServer.frame_post_draw
        var path := OS.get_environment("FORMFACTOR_CAPTURE_PATH")
        check(not path.is_empty(), "capture output path is required")
        if not path.is_empty():
            check(root.get_texture().get_image().save_jpg(path, 0.92) == OK, "runtime screenshot save failed")
    print("FormFactor 0.135: ", "PASS" if failures.is_empty() else "FAIL", " (wordless HUD, 25 parts, geometry, snapping, repeat build, history, UNKNOWN)")
    quit(0 if failures.is_empty() else 1)
