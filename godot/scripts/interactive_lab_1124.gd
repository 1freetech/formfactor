extends "res://scripts/interactive_lab_1123.gd"

# 1.124 adapts the supplied Three.js particle-emitter and selection-box ideas to
# the Godot runtime that actually ships FormFactor. Visual feedback never
# changes electrical validity, UNKNOWN handling, or engineering-core outcomes.
const PARTICLE_POOL_SIZE := 6
const PARTICLE_MAX_BURST := 64
const HIGHLIGHT_PADDING := Vector3(0.12, 0.12, 0.12)
const HIGHLIGHT_THICKNESS := 0.022

var operation_emitters: Array[GPUParticles3D] = []
var operation_emitter_index := 0
var operation_burst_count := 0

var selection_highlight: Node3D
var selection_edges: Array[MeshInstance3D] = []
var selection_material: StandardMaterial3D
var selection_target: StaticBody3D = null
var selection_size := Vector3.ZERO

func _ready() -> void:
    super._ready()
    _build_gpu_particle_pool()
    _build_selection_highlight()
    if status_label != null:
        status_label.text = "Blank board ready. Build / Buy now has GPU operation bursts and a pulsing 3D machine highlight."
    if build_status != null:
        build_status.text = "BUILD / BUY: drag to place • hover/select = 3D highlight"

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.123"):
                label.text = "FORMFACTOR 1.124 // GPU FEEDBACK BUILD LAB"
    for child in find_children("*", "Label3D", true, false):
        var label3d := child as Label3D
        if label3d != null and label3d.text.begins_with("1.123 //"):
            label3d.text = "1.124 // BUILD • HOVER • PROCESS • GLOW"

func _process(delta_seconds: float) -> void:
    super._process(delta_seconds)
    _refresh_selection_highlight()

func _build_gpu_particle_pool() -> void:
    operation_emitters.clear()
    for index in range(PARTICLE_POOL_SIZE):
        var emitter := GPUParticles3D.new()
        emitter.name = "OperationBurst3D_%d" % (index + 1)
        emitter.amount = 32
        emitter.lifetime = 0.72
        emitter.one_shot = true
        emitter.explosiveness = 1.0
        emitter.randomness = 0.28
        emitter.local_coords = false
        emitter.emitting = false
        emitter.visibility_aabb = AABB(Vector3(-3.0, -1.0, -3.0), Vector3(6.0, 6.0, 6.0))

        var process := ParticleProcessMaterial.new()
        process.direction = Vector3(0.0, 1.0, 0.0)
        process.spread = 78.0
        process.initial_velocity_min = 1.25
        process.initial_velocity_max = 2.75
        process.gravity = Vector3(0.0, -2.1, 0.0)
        process.scale_min = 0.65
        process.scale_max = 1.25
        process.color = Color(0.0, 0.9, 1.0, 1.0)

        var fade_gradient := Gradient.new()
        fade_gradient.offsets = PackedFloat32Array([0.0, 0.62, 1.0])
        fade_gradient.colors = PackedColorArray([
            Color(1.0, 1.0, 1.0, 0.96),
            Color(1.0, 1.0, 1.0, 0.74),
            Color(1.0, 1.0, 1.0, 0.0)
        ])
        var fade_texture := GradientTexture1D.new()
        fade_texture.gradient = fade_gradient
        process.color_ramp = fade_texture
        emitter.process_material = process

        var particle_mesh := QuadMesh.new()
        particle_mesh.size = Vector2(0.075, 0.075)
        var particle_material := StandardMaterial3D.new()
        particle_material.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
        particle_material.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
        particle_material.billboard_mode = BaseMaterial3D.BILLBOARD_PARTICLES
        particle_material.billboard_keep_scale = true
        particle_material.vertex_color_use_as_albedo = true
        particle_material.albedo_color = Color.WHITE
        particle_mesh.material = particle_material
        emitter.draw_pass_1 = particle_mesh

        add_child(emitter)
        operation_emitters.append(emitter)

func _emit_operation_burst(world_position: Vector3, burst_amount: int, burst_color: Color) -> void:
    if operation_emitters.is_empty():
        return
    var emitter := operation_emitters[operation_emitter_index]
    operation_emitter_index = (operation_emitter_index + 1) % operation_emitters.size()
    emitter.global_position = world_position
    emitter.amount = clampi(burst_amount, 1, PARTICLE_MAX_BURST)
    var process := emitter.process_material as ParticleProcessMaterial
    if process != null:
        process.color = burst_color
    emitter.restart()
    emitter.emitting = true
    operation_burst_count += 1

func _place_component_at_world(world_pos: Vector3) -> StaticBody3D:
    var body := super._place_component_at_world(world_pos)
    if body != null:
        _emit_operation_burst(body.global_position + Vector3(0.0, 0.62, 0.0), 28, Color(0.0, 0.90, 1.0, 1.0))
    return body

func _wire_click(screen_pos: Vector2) -> void:
    var before := user_wires.size()
    super._wire_click(screen_pos)
    if user_wires.size() <= before:
        return
    var wire: Dictionary = user_wires[user_wires.size() - 1]
    var a := wire.get("a") as StaticBody3D
    var b := wire.get("b") as StaticBody3D
    if a != null and b != null:
        var midpoint := (a.global_position + b.global_position) * 0.5 + Vector3(0.0, 0.68, 0.0)
        _emit_operation_burst(midpoint, 20, Color(1.0, 0.55, 0.08, 1.0))

func _test_player_circuit() -> void:
    super._test_player_circuit()
    if build_status == null or build_status.text != "TEST: circuit activity visible.":
        return
    for body in placed_components:
        if body != null and is_instance_valid(body) and str(body.get_meta("component_id", "")) == "led":
            _emit_operation_burst(body.global_position + Vector3(0.0, 0.78, 0.0), 44, Color(0.22, 1.0, 0.08, 1.0))

func _build_selection_highlight() -> void:
    selection_highlight = Node3D.new()
    selection_highlight.name = "SelectionBounds3D"
    selection_highlight.visible = false
    add_child(selection_highlight)

    selection_material = StandardMaterial3D.new()
    selection_material.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
    selection_material.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
    selection_material.albedo_color = Color(0.0, 1.0, 0.53, 0.82)
    selection_material.emission_enabled = true
    selection_material.emission = Color(0.0, 1.0, 0.53, 1.0)
    selection_material.emission_energy_multiplier = 2.5

    for index in range(12):
        var edge := MeshInstance3D.new()
        edge.name = "SelectionEdge_%02d" % (index + 1)
        var mesh := BoxMesh.new()
        mesh.size = Vector3(HIGHLIGHT_THICKNESS, HIGHLIGHT_THICKNESS, HIGHLIGHT_THICKNESS)
        edge.mesh = mesh
        edge.material_override = selection_material
        selection_highlight.add_child(edge)
        selection_edges.append(edge)

func _refresh_selection_highlight() -> void:
    if selection_highlight == null:
        return
    var target := hovered_component
    if target == null or not is_instance_valid(target):
        target = active_component
    if target == null or not is_instance_valid(target):
        selection_target = null
        selection_highlight.visible = false
        return

    var bounds := _target_collision_bounds(target)
    var center: Vector3 = bounds.get("center", Vector3(0.0, 0.28, 0.0))
    var size: Vector3 = bounds.get("size", Vector3(0.84, 0.68, 0.74)) + HIGHLIGHT_PADDING
    selection_target = target
    selection_highlight.visible = true
    selection_highlight.position = target.position + center
    selection_highlight.rotation = target.rotation
    if not size.is_equal_approx(selection_size):
        selection_size = size
        _resize_selection_box(size)

    var hover_is_target := hovered_component != null and is_instance_valid(hovered_component) and hovered_component == target
    var base_color := Color(0.0, 0.90, 1.0, 1.0) if hover_is_target else Color(0.22, 1.0, 0.08, 1.0)
    var pulse := 0.68 + sin(float(Time.get_ticks_msec()) * 0.009) * 0.18
    selection_material.albedo_color = Color(base_color.r, base_color.g, base_color.b, pulse)
    selection_material.emission = base_color
    selection_material.emission_energy_multiplier = 2.3 + pulse * 1.7

func _target_collision_bounds(target: StaticBody3D) -> Dictionary:
    for child in target.get_children():
        var collision := child as CollisionShape3D
        if collision == null or collision.shape == null:
            continue
        if collision.shape is BoxShape3D:
            var box := collision.shape as BoxShape3D
            return {"center": collision.position, "size": box.size}
    return {"center": Vector3(0.0, 0.28, 0.0), "size": Vector3(0.72, 0.55, 0.62)}

func _resize_selection_box(size: Vector3) -> void:
    if selection_edges.size() != 12:
        return
    var hx := size.x * 0.5
    var hy := size.y * 0.5
    var hz := size.z * 0.5
    var edge_index := 0

    for y_sign in [-1.0, 1.0]:
        for z_sign in [-1.0, 1.0]:
            _set_selection_edge(edge_index, Vector3(0.0, hy * y_sign, hz * z_sign), Vector3(size.x, HIGHLIGHT_THICKNESS, HIGHLIGHT_THICKNESS))
            edge_index += 1
    for x_sign in [-1.0, 1.0]:
        for z_sign in [-1.0, 1.0]:
            _set_selection_edge(edge_index, Vector3(hx * x_sign, 0.0, hz * z_sign), Vector3(HIGHLIGHT_THICKNESS, size.y, HIGHLIGHT_THICKNESS))
            edge_index += 1
    for x_sign in [-1.0, 1.0]:
        for y_sign in [-1.0, 1.0]:
            _set_selection_edge(edge_index, Vector3(hx * x_sign, hy * y_sign, 0.0), Vector3(HIGHLIGHT_THICKNESS, HIGHLIGHT_THICKNESS, size.z))
            edge_index += 1

func _set_selection_edge(index: int, edge_position: Vector3, edge_size: Vector3) -> void:
    var edge := selection_edges[index]
    edge.position = edge_position
    var mesh := edge.mesh as BoxMesh
    if mesh != null:
        mesh.size = edge_size

func debug_gpu_particle_pool_size() -> int:
    return operation_emitters.size()

func debug_operation_burst_count() -> int:
    return operation_burst_count

func debug_selection_edge_count() -> int:
    return selection_edges.size()

func debug_force_visual_feedback_update() -> void:
    _refresh_selection_highlight()

func debug_selection_highlight_visible() -> bool:
    return selection_highlight != null and selection_highlight.visible and selection_target != null

func debug_selection_highlight_size() -> Vector3:
    return selection_size
