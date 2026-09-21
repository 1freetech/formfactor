extends SceneTree

const EPS := 0.0005
func fail(message: String) -> void:
    push_error("0.134 visual-realism regression: " + message)
    quit(1)
func near(a: float, b: float, tolerance: float = EPS) -> bool:
    return absf(a - b) <= tolerance

func _initialize() -> void:
    var packed := load("res://scenes/main.tscn") as PackedScene
    if packed == null:
        fail("main scene missing"); return
    var lab := packed.instantiate()
    root.add_child(lab)
    await process_frame
    if not lab.has_method("mm") or not near(float(lab.mm(10.0)), 1.0):
        fail("shared mm-to-world conversion is not 0.1 world/mm"); return
    if not near(float(lab.BOARD_THICKNESS_MM), 1.6):
        fail("board thickness is not 1.6 mm"); return
    var substrate := lab.get_node_or_null("PCB_Substrate") as MeshInstance3D
    if substrate == null or not (substrate.mesh is BoxMesh):
        fail("dimensioned PCB substrate missing"); return
    var board_size := (substrate.mesh as BoxMesh).size
    if not (near(board_size.x, 8.0) and near(board_size.y, 0.16) and near(board_size.z, 5.0)):
        fail("PCB is not 80 x 50 x 1.6 mm through shared scale"); return
    if not near(substrate.position.y + board_size.y * 0.5, float(lab.BOARD_TOP_Y)):
        fail("board top plane is inconsistent"); return

    lab.select_component("resistor")
    var resistor := lab._place_component_at_world(Vector3(0, float(lab.BOARD_TOP_Y), 0))
    await process_frame
    if resistor == null:
        fail("could not place resistor"); return
    var core := resistor.get_node_or_null("AxialResistorBody3D") as MeshInstance3D
    if core == null or not (core.mesh is CylinderMesh) or not near((core.mesh as CylinderMesh).height, 0.63, 0.002):
        fail("generic resistor body is not 6.3 mm long"); return

    lab.select_component("logic")
    var dip := lab._place_component_at_world(Vector3(1.5, float(lab.BOARD_TOP_Y), 0))
    await process_frame
    if dip == null:
        fail("could not place DIP"); return
    var pins := dip.find_children("DIPPin3D", "MeshInstance3D", true, false)
    var xs: Array[float] = []
    for pin in pins:
        var x := (pin as MeshInstance3D).position.x
        if not xs.has(x): xs.append(x)
    xs.sort()
    if xs.size() < 2 or not near(xs[1] - xs[0], 0.254, 0.002):
        fail("DIP pin pitch is not 2.54 mm"); return
    if lab.get_script().resource_path.find("1134") == -1:
        fail("main scene is not using the 0.134 realism layer"); return
    if lab.inspection_toolbar_visible or lab.compact_toolbar.visible:
        fail("secondary workbench controls must start collapsed"); return
    if lab.workbench_menu_button == null or lab.workbench_menu_button.text != "WORKBENCH ▸":
        fail("collapsed Workbench menu control missing"); return
    if lab.view_hint == null or not lab.view_hint.visible:
        fail("Help must remain visible at startup"); return
    print("FormFactor 0.134 visual-realism regression passed: mm scale, board thickness/top plane, resistor body, DIP pitch, real 3D main scene.")
    quit(0)
