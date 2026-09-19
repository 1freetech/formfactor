extends SceneTree

var failures: Array[String] = []

func _initialize() -> void:
    call_deferred("_run_validation")

func _check(condition: bool, message: String) -> void:
    if not condition:
        failures.append(message)
        push_error("1.128 PBR board validation: %s" % message)

func _run_validation() -> void:
    _check(str(ProjectSettings.get_setting("application/config/name", "")) == "FormFactor 1.128",
        "application version must be FormFactor 1.128")
    _check(int(ProjectSettings.get_setting("rendering/anti_aliasing/quality/msaa_3d", 0)) == 2,
        "real 3D MSAA must be enabled")

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
        "debug_primary_board_is_3d",
        "debug_schematic_is_2d_mirror",
        "debug_pbr_board_ready",
        "debug_pbr_detail_mesh_count",
        "debug_plated_via_count",
        "debug_board_layers_are_separate"
    ]:
        _check(lab.has_method(method_name), "missing runtime gate: %s" % method_name)

    if failures.is_empty():
        _check(bool(lab.debug_primary_board_is_3d()), "physical board must remain real perspective 3D")
        _check(bool(lab.debug_schematic_is_2d_mirror()), "schematic must remain a separate 2D mirror")
        _check(bool(lab.debug_pbr_board_ready()), "PCB must use the clear-coated PBR material and rim light")
        _check(int(lab.debug_pbr_detail_mesh_count()) >= 37,
            "layered board must add at least 37 real MeshInstance3D details")
        _check(int(lab.debug_plated_via_count()) == 12,
            "board must show exactly 12 visual plated-through-hole annuli")
        _check(bool(lab.debug_board_layers_are_separate()),
            "bottom copper, FR4 core, and top mask must occupy separate 3D heights")

    var detail_root := lab.get_node_or_null("PBRBoardDetails3D") as Node3D
    _check(detail_root != null, "PBR board detail root is missing")
    var metallic_count := 0
    if detail_root != null:
        for child in detail_root.find_children("*", "MeshInstance3D", true, false):
            var mesh := child as MeshInstance3D
            _check(mesh.mesh != null, "%s must carry real mesh geometry" % mesh.name)
            _check(mesh.material_override is StandardMaterial3D,
                "%s must carry a StandardMaterial3D" % mesh.name)
            if mesh.material_override is StandardMaterial3D:
                var material := mesh.material_override as StandardMaterial3D
                if material.metallic >= 0.80:
                    metallic_count += 1
    _check(metallic_count >= 20, "copper and plated details must visibly respond as metal")

    var rim := lab.get_node_or_null("BoardRimLight3D") as SpotLight3D
    _check(rim != null and rim.shadow_enabled and rim.light_energy > 0.0,
        "real shadow-casting 3D rim light must be active")

    lab.queue_free()
    await process_frame

    if failures.is_empty():
        print("FormFactor 1.128 PBR board validation: PASS")
        quit(0)
    else:
        print("FormFactor 1.128 PBR board validation: FAIL (%d)" % failures.size())
        quit(1)
