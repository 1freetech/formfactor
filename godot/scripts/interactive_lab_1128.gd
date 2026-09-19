extends "res://scripts/interactive_lab_1127.gd"

# FormFactor 1.128
# Player-visible 3D presentation only. These meshes and materials do not provide
# electrical, thermal, package, stackup, or manufacturing evidence.

const MASK_GREEN := Color(0.018, 0.30, 0.13, 1.0)
const FR4_EDGE := Color(0.54, 0.38, 0.16, 1.0)
const COPPER_METAL := Color(0.78, 0.34, 0.09, 1.0)
const ENIG_GOLD := Color(0.88, 0.66, 0.18, 1.0)
const HOLE_DARK := Color(0.006, 0.009, 0.011, 1.0)

var pbr_board_details: Node3D
var pbr_rim_light: SpotLight3D

func _ready() -> void:
    super._ready()
    _upgrade_primary_board_material()
    _build_layered_board_details()
    _build_inspection_rim_light()
    if status_label != null:
        status_label.text = "3D PCB ready. The board now has layered edges, plated holes, fiducials, and clearer PBR lighting."
    if build_status != null:
        build_status.text = "3D BUILD // PBR BOARD"

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.127"):
                label.text = "FORMFACTOR 1.128"
    var version_label := get_node_or_null("WorkbenchVersion3D") as Label3D
    if version_label != null:
        version_label.text = "1.128 // LAYERED PBR PCB"

func _pbr_material(
        color: Color,
        metallic: float,
        roughness: float,
        clearcoat_amount: float = 0.0) -> StandardMaterial3D:
    var material := StandardMaterial3D.new()
    material.albedo_color = color
    material.metallic = metallic
    material.roughness = roughness
    if clearcoat_amount > 0.0:
        material.clearcoat_enabled = true
        material.clearcoat = clearcoat_amount
        material.clearcoat_roughness = 0.18
    return material

func _upgrade_primary_board_material() -> void:
    var pcb := get_node_or_null("PCB_Substrate") as MeshInstance3D
    if pcb == null:
        return
    var board_material := pcb.material_override as StandardMaterial3D
    if board_material == null:
        board_material = _pbr_material(MASK_GREEN, 0.02, 0.26, 0.72)
        pcb.material_override = board_material
    else:
        board_material.albedo_color = MASK_GREEN
        board_material.metallic = 0.02
        board_material.roughness = 0.26
        board_material.clearcoat_enabled = true
        board_material.clearcoat = 0.72
        board_material.clearcoat_roughness = 0.18

func _build_layered_board_details() -> void:
    pbr_board_details = Node3D.new()
    pbr_board_details.name = "PBRBoardDetails3D"
    add_child(pbr_board_details)

    var core_material := _pbr_material(FR4_EDGE, 0.0, 0.72)
    var copper_material := _pbr_material(COPPER_METAL, 0.86, 0.20)
    var gold_material := _pbr_material(ENIG_GOLD, 0.90, 0.16)
    var mask_material := _pbr_material(MASK_GREEN, 0.02, 0.24, 0.76)
    var hole_material := _pbr_material(HOLE_DARK, 0.0, 0.92)

    # Separate layers make board depth visible at normal orbit angles.
    _detail_box("FR4CoreLayer3D", Vector3(8.54, 0.050, 5.19), Vector3(0.0, -0.073, 0.0), core_material)
    _detail_box("BottomCopperLayer3D", Vector3(8.56, 0.012, 5.21), Vector3(0.0, -0.104, 0.0), copper_material)
    _detail_box("TopSolderMaskSkin3D", Vector3(8.50, 0.012, 5.15), Vector3(0.0, 0.146, 0.0), mask_material)

    _detail_box("CopperEdgeNorth3D", Vector3(8.48, 0.014, 0.035), Vector3(0.0, 0.157, -2.555), copper_material)
    _detail_box("CopperEdgeSouth3D", Vector3(8.48, 0.014, 0.035), Vector3(0.0, 0.157, 2.555), copper_material)
    _detail_box("CopperEdgeWest3D", Vector3(0.035, 0.014, 5.08), Vector3(-4.205, 0.157, 0.0), copper_material)
    _detail_box("CopperEdgeEast3D", Vector3(0.035, 0.014, 5.08), Vector3(4.205, 0.157, 0.0), copper_material)

    # A small visual prototyping row leaves the main placement area open.
    var via_x_positions: Array[float] = [-3.45, -2.75, -2.05, 2.05, 2.75, 3.45]
    for z_position: float in [-1.88, 1.88]:
        for x_position: float in via_x_positions:
            var suffix := "%d_%d" % [roundi((x_position + 4.0) * 100.0), roundi((z_position + 2.5) * 100.0)]
            _detail_cylinder("PlatedViaAnnulus3D_" + suffix, 0.115, 0.026,
                Vector3(x_position, 0.169, z_position), gold_material)
            _detail_cylinder("PlatedViaHole3D_" + suffix, 0.052, 0.038,
                Vector3(x_position, 0.176, z_position), hole_material)

    # Three copper fiducials make camera motion and surface reflection easy to read.
    var fiducials: Array[Vector3] = [
        Vector3(-3.72, 0.168, -2.13),
        Vector3(3.72, 0.168, -2.13),
        Vector3(3.72, 0.168, 2.13)
    ]
    for index: int in range(fiducials.size()):
        var position := fiducials[index]
        _detail_cylinder("FiducialClearance3D_%d" % index, 0.155, 0.018,
            position + Vector3(0.0, -0.007, 0.0), hole_material)
        _detail_cylinder("FiducialCopper3D_%d" % index, 0.082, 0.024,
            position, gold_material)

func _detail_box(
        node_name: String,
        size: Vector3,
        position: Vector3,
        material: StandardMaterial3D) -> MeshInstance3D:
    var instance := MeshInstance3D.new()
    instance.name = node_name
    var mesh := BoxMesh.new()
    mesh.size = size
    instance.mesh = mesh
    instance.position = position
    instance.material_override = material
    pbr_board_details.add_child(instance)
    return instance

func _detail_cylinder(
        node_name: String,
        radius: float,
        height: float,
        position: Vector3,
        material: StandardMaterial3D) -> MeshInstance3D:
    var instance := MeshInstance3D.new()
    instance.name = node_name
    var mesh := CylinderMesh.new()
    mesh.top_radius = radius
    mesh.bottom_radius = radius
    mesh.height = height
    mesh.radial_segments = 32
    instance.mesh = mesh
    instance.position = position
    instance.material_override = material
    pbr_board_details.add_child(instance)
    return instance

func _build_inspection_rim_light() -> void:
    pbr_rim_light = SpotLight3D.new()
    pbr_rim_light.name = "BoardRimLight3D"
    pbr_rim_light.position = Vector3(4.6, 5.2, 3.8)
    pbr_rim_light.light_color = Color(1.0, 0.72, 0.40, 1.0)
    pbr_rim_light.light_energy = 4.2
    pbr_rim_light.spot_range = 14.0
    pbr_rim_light.spot_angle = 48.0
    pbr_rim_light.shadow_enabled = true
    add_child(pbr_rim_light)
    pbr_rim_light.look_at(Vector3.ZERO, Vector3.UP)

func debug_pbr_board_ready() -> bool:
    var pcb := get_node_or_null("PCB_Substrate") as MeshInstance3D
    if pcb == null or pbr_board_details == null or pbr_rim_light == null:
        return false
    var material := pcb.material_override as StandardMaterial3D
    return material != null and material.clearcoat_enabled and material.clearcoat >= 0.70

func debug_pbr_detail_mesh_count() -> int:
    if pbr_board_details == null:
        return 0
    return pbr_board_details.find_children("*", "MeshInstance3D", true, false).size()

func debug_plated_via_count() -> int:
    if pbr_board_details == null:
        return 0
    var count := 0
    for child in pbr_board_details.find_children("*", "MeshInstance3D", true, false):
        if str(child.name).begins_with("PlatedViaAnnulus3D_"):
            count += 1
    return count

func debug_board_layers_are_separate() -> bool:
    var core := get_node_or_null("PBRBoardDetails3D/FR4CoreLayer3D") as MeshInstance3D
    var bottom_copper := get_node_or_null("PBRBoardDetails3D/BottomCopperLayer3D") as MeshInstance3D
    var top_mask := get_node_or_null("PBRBoardDetails3D/TopSolderMaskSkin3D") as MeshInstance3D
    return (
        core != null
        and bottom_copper != null
        and top_mask != null
        and bottom_copper.position.y < core.position.y
        and core.position.y < top_mask.position.y
    )
