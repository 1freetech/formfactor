extends SceneTree

var failures: Array[String] = []

func _initialize() -> void:
    call_deferred("_run_validation")

func _check(condition: bool, message: String) -> void:
    if not condition:
        failures.append(message)
        push_error("1.124 gameplay validation: %s" % message)

func _run_validation() -> void:
    var packed := load("res://scenes/main.tscn") as PackedScene
    _check(packed != null, "main scene must load")
    if packed == null:
        quit(1)
        return

    var lab := packed.instantiate()
    root.add_child(lab)
    await process_frame
    await process_frame

    _check(lab.has_method("debug_gpu_particle_pool_size"), "1.124 runtime must expose GPU particle validation")
    _check(lab.has_method("debug_selection_edge_count"), "1.124 runtime must expose selection-bound validation")
    if failures.size() > 0:
        lab.queue_free()
        quit(1)
        return

    _check(int(lab.debug_gpu_particle_pool_size()) == 6, "GPU particle pool must contain six reusable emitters")
    _check(int(lab.debug_selection_edge_count()) == 12, "3D selection box must contain twelve visible edges")

    var bursts_before: int = int(lab.debug_operation_burst_count())
    _check(bool(lab.debug_catalog_place("resistor", Vector3(0.0, 0.24, 0.0))), "catalog placement must still place a resistor on the blank PCB")
    await process_frame
    _check(int(lab.debug_operation_burst_count()) > bursts_before, "successful component placement must trigger a GPU particle burst")

    _check(bool(lab.debug_set_hover_by_refdes("R1")), "placed resistor must be available to hover targeting")
    lab.debug_force_visual_feedback_update()
    _check(bool(lab.debug_selection_highlight_visible()), "hovered component must show the 3D bounds highlight")
    var bounds: Vector3 = lab.debug_selection_highlight_size()
    _check(bounds.x > 0.72 and bounds.y > 0.55 and bounds.z > 0.62, "selection bounds must pad the actual component collision volume")

    _check(bool(lab.debug_start_board_empty()) == false, "visual feedback must not hide or delete the placed component")
    _check(int(lab.debug_wire_count()) == 0, "visual feedback validation must not invent electrical connections")

    lab.queue_free()
    await process_frame

    if failures.is_empty():
        print("FormFactor 1.124 GPU feedback validation: PASS")
        quit(0)
    else:
        print("FormFactor 1.124 GPU feedback validation: FAIL (%d)" % failures.size())
        quit(1)
