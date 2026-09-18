extends SceneTree

var failures: Array[String] = []

func _initialize() -> void:
    call_deferred("_run_validation")

func _check(condition: bool, message: String) -> void:
    if not condition:
        failures.append(message)
        push_error("1.127 pin-graph validation: %s" % message)

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

    for method_name in [
        "debug_catalog_place",
        "debug_find_component",
        "debug_pin_catalog_valid",
        "debug_pin_count_for",
        "debug_connect_pin_refs",
        "debug_wire_pin_pair",
        "debug_named_net_count",
        "debug_snapshot_exported",
        "debug_last_pin_precheck_passed",
        "debug_truth_gate_count"
    ]:
        _check(lab.has_method(method_name), "missing 1.127 validation method: %s" % method_name)

    if not failures.is_empty():
        lab.queue_free()
        quit(1)
        return

    _check(bool(lab.debug_pin_catalog_valid()), "pin catalogue must pass fail-closed validation")
    _check(int(lab.debug_pin_count_for("resistor")) == 2, "resistor must expose two pins")
    _check(int(lab.debug_pin_count_for("logic")) == 3, "logic block must expose A, B, and Y")
    _check(int(lab.debug_pin_count_for("relay")) == 5, "relay must expose five named pins")
    _check(int(lab.debug_truth_gate_count()) == 7, "truth-table preview must expose seven supported gate families")

    lab.call("_clear_player_board")
    _check(bool(lab.debug_catalog_place("power", Vector3(-2.0, 0.24, 0.0))), "must place power source")
    _check(bool(lab.debug_catalog_place("resistor", Vector3(0.0, 0.24, 0.0))), "must place resistor")
    _check(bool(lab.debug_catalog_place("led", Vector3(2.0, 0.24, 0.0))), "must place LED")

    _check(bool(lab.debug_connect_pin_refs("PWR1", "POS", "R1", "1")), "must wire PWR1.POS to R1.1")
    _check(bool(lab.debug_connect_pin_refs("R1", "2", "D1", "A")), "must wire R1.2 to D1.A")
    _check(bool(lab.debug_connect_pin_refs("D1", "K", "PWR1", "NEG")), "must wire D1.K to PWR1.NEG")

    await process_frame
    await process_frame

    _check(int(lab.debug_wire_count()) == 3, "board must retain all three exact pin wires")
    _check(lab.debug_wire_pin_pair(0) == PackedStringArray(["POS", "1"]), "first wire must preserve exact pin ids")
    _check(lab.debug_wire_pin_pair(1) == PackedStringArray(["2", "A"]), "second wire must preserve exact pin ids")
    _check(lab.debug_wire_pin_pair(2) == PackedStringArray(["K", "NEG"]), "third wire must preserve exact pin ids")
    _check(int(lab.debug_named_net_count()) == 3, "three external pin wires should form three named nets")

    var resistor := lab.debug_find_component("R1") as StaticBody3D
    _check(resistor != null, "R1 must remain addressable by reference designator")
    if resistor != null:
        _check(str(resistor.get_meta("symbol_id", "")) == "Device:R", "R1 must retain symbol metadata")
        _check(str(resistor.get_meta("footprint_id", "")) == "Resistor_SMD:R_0402_1005Metric", "R1 must retain reference footprint metadata")

    var canvas := lab.get("schematic_canvas") as Control
    _check(canvas != null, "1.127 vector schematic canvas must exist")
    if canvas != null:
        _check(canvas.has_method("debug_pin_labels_present"), "schematic canvas must expose pin-label validation")
        if canvas.has_method("debug_pin_labels_present"):
            _check(bool(canvas.call("debug_pin_labels_present")), "schematic must preserve exact wire pin labels")

    lab.call("_test_player_circuit")
    _check(bool(lab.debug_last_pin_precheck_passed()), "closed + -> LED A and LED K -> - pin loop must pass topology precheck")
    _check(bool(lab.debug_snapshot_exported()), "editor must export deterministic engineering snapshot")

    var snapshot_path := "user://last_engineering_board_1_127.json"
    var snapshot_file := FileAccess.open(snapshot_path, FileAccess.READ)
    _check(snapshot_file != null, "engineering snapshot file must be readable")
    if snapshot_file != null:
        var snapshot = JSON.parse_string(snapshot_file.get_as_text())
        snapshot_file.close()
        _check(snapshot is Dictionary, "engineering snapshot must be JSON object")
        if snapshot is Dictionary:
            var snapshot_dict := snapshot as Dictionary
            _check(str(snapshot_dict.get("format", "")) == "formfactor-editor-snapshot-v1", "snapshot format must be versioned")
            _check(str(snapshot_dict.get("engineering_truth", "")) == "cpp_core_required", "snapshot must state that C++ core is authoritative")
            _check((snapshot_dict.get("wires", []) as Array).size() == 3, "snapshot must contain three exact pin wires")

    lab.call("_save_blueprint")
    lab.call("_clear_player_board")
    _check(int(lab.debug_placed_count()) == 0, "clear must empty the board before blueprint restore")
    lab.call("_load_blueprint")
    await process_frame
    _check(int(lab.debug_placed_count()) == 3, "blueprint restore must rebuild all parts")
    _check(int(lab.debug_wire_count()) == 3, "blueprint restore must rebuild all pin wires")
    lab.call("_test_player_circuit")
    _check(bool(lab.debug_last_pin_precheck_passed()), "restored blueprint must preserve the pin-level loop")

    lab.queue_free()
    await process_frame

    if failures.is_empty():
        print("FormFactor 1.127 pin-graph validation: PASS")
        quit(0)
    else:
        print("FormFactor 1.127 pin-graph validation: FAIL (%d)" % failures.size())
        quit(1)
