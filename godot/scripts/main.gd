extends Node3D

const NEON := Color(0.22, 1.0, 0.08, 1.0)
const BOARD_GREEN := Color(0.025, 0.24, 0.12, 1.0)
const COPPER := Color(0.72, 0.34, 0.10, 1.0)
const DARK_METAL := Color(0.055, 0.065, 0.075, 1.0)
const WHITE := Color(0.90, 0.95, 0.96, 1.0)

var camera_pivot: Node3D
var camera: Camera3D
var status_label: Label
var step_label: Label
var led_material: StandardMaterial3D
var led_glow_material: StandardMaterial3D
var led_mesh: MeshInstance3D
var current_pulse: MeshInstance3D
var pulse_start := Vector3(-2.3, 0.72, 0.0)
var pulse_end := Vector3(2.3, 0.72, 0.0)
var circuit_live := false
var tutorial_step := 0
var orbiting := false
var last_mouse := Vector2.ZERO

func _ready() -> void:
    _build_environment()
    _build_workbench()
    _build_board()
    _build_power_source()
    _build_led()
    _build_trace_path()
    _build_camera()
    _build_ui()
    _set_step(0)

func _process(delta: float) -> void:
    if circuit_live and current_pulse:
        var t := fmod(Time.get_ticks_msec() * 0.00045, 1.0)
        current_pulse.position = pulse_start.lerp(pulse_end, t)
        current_pulse.rotate_y(delta * 2.5)

func _unhandled_input(event: InputEvent) -> void:
    if event is InputEventMouseButton:
        if event.button_index == MOUSE_BUTTON_RIGHT or event.button_index == MOUSE_BUTTON_MIDDLE:
            orbiting = event.pressed
            last_mouse = event.position
            get_viewport().set_input_as_handled()
            return
        if event.button_index == MOUSE_BUTTON_WHEEL_UP and event.pressed:
            camera.position.z = max(4.8, camera.position.z - 0.45)
            get_viewport().set_input_as_handled()
            return
        if event.button_index == MOUSE_BUTTON_WHEEL_DOWN and event.pressed:
            camera.position.z = min(11.5, camera.position.z + 0.45)
            get_viewport().set_input_as_handled()
            return
        if event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
            _pick_part(event.position)
            get_viewport().set_input_as_handled()
            return
    elif event is InputEventMouseMotion and orbiting:
        var delta := event.position - last_mouse
        last_mouse = event.position
        camera_pivot.rotation.y -= delta.x * 0.007
        camera_pivot.rotation.x = clamp(camera_pivot.rotation.x - delta.y * 0.005, -0.35, 0.75)
        get_viewport().set_input_as_handled()
        return
    elif event is InputEventKey and event.pressed:
        match event.keycode:
            KEY_R:
                camera_pivot.rotation = Vector3(-0.18, -0.18, 0.0)
                camera.position = Vector3(0.0, 4.8, 8.4)
            KEY_C:
                _reset_tutorial()
            KEY_ESCAPE:
                get_tree().quit()

func _build_environment() -> void:
    var world := WorldEnvironment.new()
    world.name = "WorldEnvironment"
    var env := Environment.new()
    env.background_mode = Environment.BG_COLOR
    env.background_color = Color(0.006, 0.01, 0.013, 1.0)
    env.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
    env.ambient_light_color = Color(0.22, 0.28, 0.30, 1.0)
    env.ambient_light_energy = 0.85
    env.tonemap_mode = Environment.TONE_MAPPER_FILMIC
    world.environment = env
    add_child(world)

    var key := DirectionalLight3D.new()
    key.name = "KeyLight3D"
    key.rotation_degrees = Vector3(-56.0, -28.0, 0.0)
    key.light_color = Color(0.92, 0.98, 1.0, 1.0)
    key.light_energy = 2.2
    key.shadow_enabled = true
    add_child(key)

    var fill := OmniLight3D.new()
    fill.name = "NeonFill3D"
    fill.position = Vector3(-3.8, 4.0, 2.6)
    fill.light_color = Color(0.16, 1.0, 0.08, 1.0)
    fill.light_energy = 5.0
    fill.omni_range = 10.0
    fill.shadow_enabled = true
    add_child(fill)

func _build_workbench() -> void:
    var floor_mat := _material(Color(0.035, 0.043, 0.050, 1.0), 0.12, 0.72)
    _box("Workbench", Vector3(14.0, 0.55, 10.0), Vector3(0.0, -0.42, 0.0), floor_mat)
    _box("BackWall", Vector3(14.0, 6.0, 0.35), Vector3(0.0, 2.4, -5.1),
        _material(Color(0.018, 0.023, 0.028, 1.0), 0.0, 0.93))

    for x in range(-5, 6):
        _box("BenchGridX_%d" % x, Vector3(0.012, 0.008, 9.2),
            Vector3(float(x), -0.125, 0.0), _material(Color(0.08, 0.11, 0.12, 1.0), 0.0, 1.0))
    for z in range(-4, 5):
        _box("BenchGridZ_%d" % z, Vector3(12.0, 0.008, 0.012),
            Vector3(0.0, -0.124, float(z)), _material(Color(0.08, 0.11, 0.12, 1.0), 0.0, 1.0))

func _build_board() -> void:
    var pcb := _material(BOARD_GREEN, 0.05, 0.46)
    _box("PCB_Substrate", Vector3(8.6, 0.20, 5.25), Vector3(0.0, 0.04, 0.0), pcb)

    var edge := _material(Color(0.08, 0.34, 0.19, 1.0), 0.06, 0.32)
    _box("PCB_EdgeTop", Vector3(8.65, 0.03, 0.035), Vector3(0.0, 0.155, -2.62), edge)
    _box("PCB_EdgeBottom", Vector3(8.65, 0.03, 0.035), Vector3(0.0, 0.155, 2.62), edge)

    for pos in [Vector3(-3.85, 0.16, -2.18), Vector3(3.85, 0.16, -2.18), Vector3(-3.85, 0.16, 2.18), Vector3(3.85, 0.16, 2.18)]:
        var ring := _cylinder("MountRing", 0.18, 0.025, pos,
            _material(Color(0.62, 0.66, 0.64, 1.0), 0.85, 0.20))
        ring.rotation_degrees.x = 90.0
        var hole := _cylinder("MountHole", 0.095, 0.032, pos + Vector3(0.0, 0.01, 0.0),
            _material(Color(0.008, 0.012, 0.013, 1.0), 0.0, 1.0))
        hole.rotation_degrees.x = 90.0

    _label3d("FORMFACTOR // TRAINING PCB", Vector3(-3.85, 0.185, 2.36), Vector3(-90.0, 0.0, 0.0), 0.15, WHITE)
    _label3d("1.06 MATERIAL LAB", Vector3(2.15, 0.185, -2.30), Vector3(-90.0, 0.0, 0.0), 0.12, NEON)

    var solder := _material(Color(0.69, 0.72, 0.74, 1.0), 0.92, 0.18)
    for x in [-2.65, -2.25, 2.10, 2.50]:
        _cylinder("SolderPad", 0.11, 0.025, Vector3(x, 0.175, 0.0), solder)

func _build_power_source() -> void:
    var body := StaticBody3D.new()
    body.name = "PowerBody3D"
    body.set_meta("part_id", "power")
    body.position = Vector3(-2.45, 0.52, 0.0)
    add_child(body)

    var mesh := MeshInstance3D.new()
    mesh.name = "PowerCaseMesh3D"
    var box_mesh := BoxMesh.new()
    box_mesh.size = Vector3(1.25, 0.65, 1.15)
    mesh.mesh = box_mesh
    mesh.material_override = _material(DARK_METAL, 0.58, 0.28)
    body.add_child(mesh)

    var collision := CollisionShape3D.new()
    var shape := BoxShape3D.new()
    shape.size = box_mesh.size
    collision.shape = shape
    body.add_child(collision)

    var terminal_metal := _material(Color(0.68, 0.70, 0.72, 1.0), 0.96, 0.12)
    var positive := _cylinder_child(body, "PositiveTerminal", 0.13, 0.16,
        Vector3(-0.28, 0.40, 0.0), terminal_metal)
    positive.rotation_degrees.x = 90.0
    var negative := _cylinder_child(body, "NegativeTerminal", 0.13, 0.16,
        Vector3(0.28, 0.40, 0.0), terminal_metal)
    negative.rotation_degrees.x = 90.0

    var plus_mat := _material(Color(0.82, 0.12, 0.08, 1.0), 0.18, 0.32)
    var minus_mat := _material(Color(0.035, 0.04, 0.045, 1.0), 0.25, 0.42)
    var plus_cap := _cylinder_child(body, "PositiveCap", 0.085, 0.04,
        Vector3(-0.28, 0.49, 0.0), plus_mat)
    plus_cap.rotation_degrees.x = 90.0
    var minus_cap := _cylinder_child(body, "NegativeCap", 0.085, 0.04,
        Vector3(0.28, 0.49, 0.0), minus_mat)
    minus_cap.rotation_degrees.x = 90.0

    _label3d_child(body, "PWR", Vector3(-0.34, 0.36, 0.59), Vector3(0.0, 0.0, 0.0), 0.16, WHITE)

func _build_led() -> void:
    var body := StaticBody3D.new()
    body.name = "LedBody3D"
    body.set_meta("part_id", "led")
    body.position = Vector3(2.35, 0.53, 0.0)
    add_child(body)

    led_material = _material(Color(0.10, 0.34, 0.08, 0.96), 0.02, 0.18)
    led_glow_material = _material(Color(0.34, 1.0, 0.20, 1.0), 0.0, 0.08, Color(0.22, 1.0, 0.08, 1.0), 5.0)

    led_mesh = MeshInstance3D.new()
    led_mesh.name = "LedDomeMesh3D"
    var sphere := SphereMesh.new()
    sphere.radius = 0.31
    sphere.height = 0.56
    sphere.radial_segments = 32
    sphere.rings = 16
    led_mesh.mesh = sphere
    led_mesh.scale = Vector3(1.0, 1.16, 1.0)
    led_mesh.position = Vector3(0.0, 0.22, 0.0)
    led_mesh.material_override = led_material
    body.add_child(led_mesh)

    var collar := CylinderMesh.new()
    collar.top_radius = 0.34
    collar.bottom_radius = 0.34
    collar.height = 0.16
    collar.radial_segments = 32
    var collar_node := MeshInstance3D.new()
    collar_node.name = "LedCollarMesh3D"
    collar_node.mesh = collar
    collar_node.position = Vector3(0.0, -0.08, 0.0)
    collar_node.material_override = _material(Color(0.06, 0.16, 0.055, 1.0), 0.04, 0.28)
    body.add_child(collar_node)

    var collision := CollisionShape3D.new()
    var shape := SphereShape3D.new()
    shape.radius = 0.38
    collision.position = Vector3(0.0, 0.18, 0.0)
    collision.shape = shape
    body.add_child(collision)

    var leg_mat := _material(Color(0.72, 0.74, 0.75, 1.0), 0.94, 0.14)
    _box_child(body, "AnodeLeg", Vector3(0.08, 0.34, 0.08), Vector3(-0.12, -0.28, 0.0), leg_mat)
    _box_child(body, "CathodeLeg", Vector3(0.08, 0.34, 0.08), Vector3(0.12, -0.28, 0.0), leg_mat)
    _label3d_child(body, "LED", Vector3(-0.34, 0.68, 0.0), Vector3(0.0, 0.0, 0.0), 0.16, WHITE)

    var glow := OmniLight3D.new()
    glow.name = "LedGlowLight3D"
    glow.position = Vector3(0.0, 0.34, 0.0)
    glow.light_color = NEON
    glow.light_energy = 0.0
    glow.omni_range = 3.3
    body.add_child(glow)

func _build_trace_path() -> void:
    var copper_mat := _material(COPPER, 0.88, 0.22)
    _box("CopperTrace_A", Vector3(1.25, 0.035, 0.12), Vector3(-1.45, 0.175, 0.0), copper_mat)
    _box("CopperTrace_B", Vector3(1.25, 0.035, 0.12), Vector3(1.45, 0.175, 0.0), copper_mat)
    _box("CopperTrace_Center", Vector3(1.75, 0.035, 0.12), Vector3(0.0, 0.175, 0.0), copper_mat)

    var wire_mat := _material(Color(0.95, 0.30, 0.04, 1.0), 0.46, 0.30)
    var wire := _segment("Wire3D", pulse_start, pulse_end, 0.055, wire_mat)
    wire.visible = false

    current_pulse = MeshInstance3D.new()
    current_pulse.name = "CurrentPulseMesh3D"
    var pulse_sphere := SphereMesh.new()
    pulse_sphere.radius = 0.095
    pulse_sphere.height = 0.19
    current_pulse.mesh = pulse_sphere
    current_pulse.material_override = _material(Color(1.0, 0.92, 0.20, 1.0), 0.0, 0.05,
        Color(1.0, 0.78, 0.08, 1.0), 4.0)
    current_pulse.visible = false
    add_child(current_pulse)

func _build_camera() -> void:
    camera_pivot = Node3D.new()
    camera_pivot.name = "CameraPivot3D"
    camera_pivot.rotation = Vector3(-0.18, -0.18, 0.0)
    add_child(camera_pivot)

    camera = Camera3D.new()
    camera.name = "Camera3D"
    camera.position = Vector3(0.0, 4.8, 8.4)
    camera.fov = 48.0
    camera.current = true
    camera_pivot.add_child(camera)
    camera.look_at_from_position(camera.position, Vector3(0.0, 0.25, 0.0), Vector3.UP)

func _build_ui() -> void:
    var layer := CanvasLayer.new()
    layer.name = "HUD"
    add_child(layer)

    var panel := ColorRect.new()
    panel.name = "InstructionPanel"
    panel.position = Vector2(18.0, 18.0)
    panel.size = Vector2(420.0, 194.0)
    panel.color = Color(0.015, 0.025, 0.030, 0.93)
    layer.add_child(panel)

    var title := Label.new()
    title.position = Vector2(20.0, 16.0)
    title.text = "FORMFACTOR 1.06 // MATERIAL 3D LAB"
    title.add_theme_color_override("font_color", NEON)
    title.add_theme_font_size_override("font_size", 20)
    panel.add_child(title)

    step_label = Label.new()
    step_label.position = Vector2(20.0, 53.0)
    step_label.size = Vector2(380.0, 54.0)
    step_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    step_label.add_theme_color_override("font_color", WHITE)
    step_label.add_theme_font_size_override("font_size", 16)
    panel.add_child(step_label)

    status_label = Label.new()
    status_label.position = Vector2(20.0, 112.0)
    status_label.size = Vector2(380.0, 60.0)
    status_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    status_label.add_theme_color_override("font_color", Color(0.66, 0.76, 0.78, 1.0))
    status_label.add_theme_font_size_override("font_size", 14)
    panel.add_child(status_label)

    var controls := Label.new()
    controls.position = Vector2(18.0, 830.0)
    controls.text = "LEFT CLICK PARTS  •  RIGHT/MIDDLE DRAG ORBIT  •  WHEEL ZOOM  •  R CAMERA  •  C RESET"
    controls.add_theme_color_override("font_color", Color(0.70, 0.78, 0.80, 1.0))
    controls.add_theme_font_size_override("font_size", 14)
    layer.add_child(controls)

    var truth := Label.new()
    truth.position = Vector2(1010.0, 18.0)
    truth.size = Vector2(440.0, 78.0)
    truth.text = "3D DISPLAY LAYER\nMaterials and animation show state only.\nEngineering truth remains in the validated core."
    truth.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
    truth.add_theme_color_override("font_color", Color(0.62, 0.73, 0.75, 1.0))
    truth.add_theme_font_size_override("font_size", 13)
    layer.add_child(truth)

func _pick_part(screen_pos: Vector2) -> void:
    if not camera:
        return
    var origin := camera.project_ray_origin(screen_pos)
    var end := origin + camera.project_ray_normal(screen_pos) * 100.0
    var query := PhysicsRayQueryParameters3D.create(origin, end)
    query.collide_with_areas = true
    query.collide_with_bodies = true
    var hit := get_world_3d().direct_space_state.intersect_ray(query)
    if hit.is_empty():
        status_label.text = "Click the physical power block or LED on the board."
        return
    var collider := hit.get("collider")
    if collider == null:
        return
    var part_id := str(collider.get_meta("part_id", ""))
    if tutorial_step == 0:
        if part_id == "power":
            _set_step(1)
        else:
            status_label.text = "Start at the power source. Click the metal power block first."
    elif tutorial_step == 1:
        if part_id == "led":
            _complete_circuit()
        else:
            status_label.text = "Power is selected. Click the LED next."

func _set_step(next_step: int) -> void:
    tutorial_step = next_step
    if next_step == 0:
        step_label.text = "STEP 1 // Click the 3D power source."
        status_label.text = "Orbit the real 3D board, inspect its materials, then choose the power source."
    elif next_step == 1:
        step_label.text = "STEP 2 // Click the 3D LED."
        status_label.text = "Power selected. Click the LED to complete the visible training path."
    else:
        step_label.text = "CIRCUIT LIVE // Watch the LED and current pulse."
        status_label.text = "Visual feedback is active because this training interaction completed."

func _complete_circuit() -> void:
    circuit_live = true
    _set_step(2)
    led_mesh.material_override = led_glow_material
    var led_body := get_node_or_null("LedBody3D")
    if led_body:
        var glow := led_body.get_node_or_null("LedGlowLight3D") as OmniLight3D
        if glow:
            glow.light_energy = 5.5
    var wire := get_node_or_null("Wire3D")
    if wire:
        wire.visible = true
    if current_pulse:
        current_pulse.visible = true

func _reset_tutorial() -> void:
    circuit_live = false
    if led_mesh:
        led_mesh.material_override = led_material
    var led_body := get_node_or_null("LedBody3D")
    if led_body:
        var glow := led_body.get_node_or_null("LedGlowLight3D") as OmniLight3D
        if glow:
            glow.light_energy = 0.0
    var wire := get_node_or_null("Wire3D")
    if wire:
        wire.visible = false
    if current_pulse:
        current_pulse.visible = false
    _set_step(0)

func _material(color: Color, metallic := 0.0, roughness := 0.5,
               emission := Color(0, 0, 0, 1), emission_energy := 0.0) -> StandardMaterial3D:
    var mat := StandardMaterial3D.new()
    mat.albedo_color = color
    mat.metallic = metallic
    mat.roughness = roughness
    if emission_energy > 0.0:
        mat.emission_enabled = true
        mat.emission = emission
        mat.emission_energy_multiplier = emission_energy
    return mat

func _box(name_text: String, size: Vector3, pos: Vector3, mat: Material) -> MeshInstance3D:
    var node := MeshInstance3D.new()
    node.name = name_text
    var mesh_data := BoxMesh.new()
    mesh_data.size = size
    node.mesh = mesh_data
    node.position = pos
    node.material_override = mat
    add_child(node)
    return node

func _box_child(parent: Node3D, name_text: String, size: Vector3, pos: Vector3,
                mat: Material) -> MeshInstance3D:
    var node := MeshInstance3D.new()
    node.name = name_text
    var mesh_data := BoxMesh.new()
    mesh_data.size = size
    node.mesh = mesh_data
    node.position = pos
    node.material_override = mat
    parent.add_child(node)
    return node

func _cylinder(name_text: String, radius: float, height: float, pos: Vector3,
               mat: Material) -> MeshInstance3D:
    var node := MeshInstance3D.new()
    node.name = name_text
    var mesh_data := CylinderMesh.new()
    mesh_data.top_radius = radius
    mesh_data.bottom_radius = radius
    mesh_data.height = height
    mesh_data.radial_segments = 28
    node.mesh = mesh_data
    node.position = pos
    node.material_override = mat
    add_child(node)
    return node

func _cylinder_child(parent: Node3D, name_text: String, radius: float, height: float,
                     pos: Vector3, mat: Material) -> MeshInstance3D:
    var node := MeshInstance3D.new()
    node.name = name_text
    var mesh_data := CylinderMesh.new()
    mesh_data.top_radius = radius
    mesh_data.bottom_radius = radius
    mesh_data.height = height
    mesh_data.radial_segments = 28
    node.mesh = mesh_data
    node.position = pos
    node.material_override = mat
    parent.add_child(node)
    return node

func _segment(name_text: String, from: Vector3, to: Vector3, radius: float,
              mat: Material) -> MeshInstance3D:
    var dir := to - from
    var node := MeshInstance3D.new()
    node.name = name_text
    var cyl := CylinderMesh.new()
    cyl.top_radius = radius
    cyl.bottom_radius = radius
    cyl.height = dir.length()
    cyl.radial_segments = 24
    node.mesh = cyl
    node.material_override = mat
    node.position = (from + to) * 0.5
    var y_axis := dir.normalized()
    var x_axis := y_axis.cross(Vector3.FORWARD)
    if x_axis.length() < 0.01:
        x_axis = y_axis.cross(Vector3.RIGHT)
    x_axis = x_axis.normalized()
    var z_axis := x_axis.cross(y_axis).normalized()
    node.basis = Basis(x_axis, y_axis, z_axis)
    add_child(node)
    return node

func _label3d(text_value: String, pos: Vector3, rot_deg: Vector3,
              pixel_size: float, color: Color) -> Label3D:
    var label := Label3D.new()
    label.name = "SilkLabel3D"
    label.text = text_value
    label.position = pos
    label.rotation_degrees = rot_deg
    label.pixel_size = pixel_size
    label.modulate = color
    label.no_depth_test = false
    add_child(label)
    return label

func _label3d_child(parent: Node3D, text_value: String, pos: Vector3, rot_deg: Vector3,
                    pixel_size: float, color: Color) -> Label3D:
    var label := Label3D.new()
    label.name = "%s_Label3D" % text_value
    label.text = text_value
    label.position = pos
    label.rotation_degrees = rot_deg
    label.pixel_size = pixel_size
    label.modulate = color
    label.no_depth_test = false
    parent.add_child(label)
    return label
