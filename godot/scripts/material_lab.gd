extends Node3D

const NEON: Color = Color(0.22, 1.0, 0.08, 1.0)
const BOARD_GREEN: Color = Color(0.025, 0.24, 0.12, 1.0)
const COPPER: Color = Color(0.72, 0.34, 0.10, 1.0)
const DARK_METAL: Color = Color(0.055, 0.065, 0.075, 1.0)
const WHITE: Color = Color(0.90, 0.95, 0.96, 1.0)

var camera_pivot: Node3D
var camera: Camera3D
var status_label: Label
var step_label: Label
var led_mesh: MeshInstance3D
var led_off_material: StandardMaterial3D
var led_on_material: StandardMaterial3D
var current_pulse: MeshInstance3D
var pulse_start: Vector3 = Vector3(-2.3, 0.72, 0.0)
var pulse_end: Vector3 = Vector3(2.3, 0.72, 0.0)
var tutorial_step: int = 0
var circuit_live: bool = false
var orbiting: bool = false
var last_mouse: Vector2 = Vector2.ZERO

func _ready() -> void:
    _build_environment()
    _build_room()
    _build_board()
    _build_power_source()
    _build_led()
    _build_copper_path()
    _build_camera()
    _build_hud()
    _set_step(0)

func _process(delta_seconds: float) -> void:
    if circuit_live and current_pulse != null:
        var t: float = fmod(float(Time.get_ticks_msec()) * 0.00045, 1.0)
        current_pulse.position = pulse_start.lerp(pulse_end, t)
        current_pulse.rotate_y(delta_seconds * 2.5)

func _unhandled_input(event: InputEvent) -> void:
    if event is InputEventMouseButton:
        var mouse_button: InputEventMouseButton = event as InputEventMouseButton
        if mouse_button.button_index == MOUSE_BUTTON_RIGHT or mouse_button.button_index == MOUSE_BUTTON_MIDDLE:
            orbiting = mouse_button.pressed
            last_mouse = mouse_button.position
            get_viewport().set_input_as_handled()
            return
        if mouse_button.button_index == MOUSE_BUTTON_WHEEL_UP and mouse_button.pressed:
            camera.position.z = maxf(4.8, camera.position.z - 0.45)
            get_viewport().set_input_as_handled()
            return
        if mouse_button.button_index == MOUSE_BUTTON_WHEEL_DOWN and mouse_button.pressed:
            camera.position.z = minf(11.5, camera.position.z + 0.45)
            get_viewport().set_input_as_handled()
            return
        if mouse_button.button_index == MOUSE_BUTTON_LEFT and mouse_button.pressed:
            _pick_part(mouse_button.position)
            get_viewport().set_input_as_handled()
            return
    elif event is InputEventMouseMotion and orbiting:
        var mouse_motion: InputEventMouseMotion = event as InputEventMouseMotion
        var mouse_delta: Vector2 = mouse_motion.position - last_mouse
        last_mouse = mouse_motion.position
        camera_pivot.rotation.y -= mouse_delta.x * 0.007
        camera_pivot.rotation.x = clampf(camera_pivot.rotation.x - mouse_delta.y * 0.005, -0.35, 0.75)
        get_viewport().set_input_as_handled()
        return
    elif event is InputEventKey:
        var key_event: InputEventKey = event as InputEventKey
        if not key_event.pressed:
            return
        if key_event.keycode == KEY_R:
            camera_pivot.rotation = Vector3(-0.18, -0.18, 0.0)
            camera.position = Vector3(0.0, 4.8, 8.4)
        elif key_event.keycode == KEY_C:
            _reset_tutorial()
        elif key_event.keycode == KEY_ESCAPE:
            get_tree().quit()

func _build_environment() -> void:
    var world: WorldEnvironment = WorldEnvironment.new()
    world.name = "WorldEnvironment"
    var environment: Environment = Environment.new()
    environment.background_mode = Environment.BG_COLOR
    environment.background_color = Color(0.006, 0.01, 0.013, 1.0)
    environment.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
    environment.ambient_light_color = Color(0.22, 0.28, 0.30, 1.0)
    environment.ambient_light_energy = 0.85
    environment.tonemap_mode = Environment.TONE_MAPPER_FILMIC
    world.environment = environment
    add_child(world)

    var key_light: DirectionalLight3D = DirectionalLight3D.new()
    key_light.name = "KeyLight3D"
    key_light.rotation_degrees = Vector3(-56.0, -28.0, 0.0)
    key_light.light_color = Color(0.92, 0.98, 1.0, 1.0)
    key_light.light_energy = 2.2
    key_light.shadow_enabled = true
    add_child(key_light)

    var fill_light: OmniLight3D = OmniLight3D.new()
    fill_light.name = "NeonFill3D"
    fill_light.position = Vector3(-3.8, 4.0, 2.6)
    fill_light.light_color = NEON
    fill_light.light_energy = 5.0
    fill_light.omni_range = 10.0
    fill_light.shadow_enabled = true
    add_child(fill_light)

func _build_room() -> void:
    var floor_material: StandardMaterial3D = _material(Color(0.035, 0.043, 0.050, 1.0), 0.12, 0.72)
    _box("Workbench", Vector3(14.0, 0.55, 10.0), Vector3(0.0, -0.42, 0.0), floor_material)
    _box("BackWall", Vector3(14.0, 6.0, 0.35), Vector3(0.0, 2.4, -5.1),
        _material(Color(0.018, 0.023, 0.028, 1.0), 0.0, 0.93))

    var grid_material: StandardMaterial3D = _material(Color(0.08, 0.11, 0.12, 1.0), 0.0, 1.0)
    for x_index: int in range(-5, 6):
        _box("BenchGridX_%d" % x_index, Vector3(0.012, 0.008, 9.2),
            Vector3(float(x_index), -0.125, 0.0), grid_material)
    for z_index: int in range(-4, 5):
        _box("BenchGridZ_%d" % z_index, Vector3(12.0, 0.008, 0.012),
            Vector3(0.0, -0.124, float(z_index)), grid_material)

func _build_board() -> void:
    _box("PCB_Substrate", Vector3(8.6, 0.20, 5.25), Vector3(0.0, 0.04, 0.0),
        _material(BOARD_GREEN, 0.05, 0.46))
    var edge_material: StandardMaterial3D = _material(Color(0.08, 0.34, 0.19, 1.0), 0.06, 0.32)
    _box("PCB_EdgeTop", Vector3(8.65, 0.03, 0.035), Vector3(0.0, 0.155, -2.62), edge_material)
    _box("PCB_EdgeBottom", Vector3(8.65, 0.03, 0.035), Vector3(0.0, 0.155, 2.62), edge_material)

    var ring_material: StandardMaterial3D = _material(Color(0.62, 0.66, 0.64, 1.0), 0.85, 0.20)
    var hole_material: StandardMaterial3D = _material(Color(0.008, 0.012, 0.013, 1.0), 0.0, 1.0)
    var hole_positions: Array[Vector3] = [
        Vector3(-3.85, 0.16, -2.18), Vector3(3.85, 0.16, -2.18),
        Vector3(-3.85, 0.16, 2.18), Vector3(3.85, 0.16, 2.18)
    ]
    for hole_position: Vector3 in hole_positions:
        _cylinder("MountRing", 0.18, 0.025, hole_position, ring_material)
        _cylinder("MountHole", 0.095, 0.032, hole_position + Vector3(0.0, 0.01, 0.0), hole_material)

    _label3d("FORMFACTOR // TRAINING PCB", Vector3(-3.85, 0.185, 2.36), 0.010, WHITE)
    _label3d("1.06 MATERIAL LAB", Vector3(2.15, 0.185, -2.30), 0.009, NEON)

    var solder_material: StandardMaterial3D = _material(Color(0.69, 0.72, 0.74, 1.0), 0.92, 0.18)
    var pad_x_positions: Array[float] = [-2.65, -2.25, 2.10, 2.50]
    for pad_x: float in pad_x_positions:
        _cylinder("SolderPad", 0.11, 0.025, Vector3(pad_x, 0.175, 0.0), solder_material)

func _build_power_source() -> void:
    var body: StaticBody3D = StaticBody3D.new()
    body.name = "PowerBody3D"
    body.set_meta("part_id", "power")
    body.position = Vector3(-2.45, 0.52, 0.0)
    add_child(body)

    var case_mesh_instance: MeshInstance3D = MeshInstance3D.new()
    case_mesh_instance.name = "PowerCaseMesh3D"
    var case_mesh: BoxMesh = BoxMesh.new()
    case_mesh.size = Vector3(1.25, 0.65, 1.15)
    case_mesh_instance.mesh = case_mesh
    case_mesh_instance.material_override = _material(DARK_METAL, 0.58, 0.28)
    body.add_child(case_mesh_instance)

    var collision: CollisionShape3D = CollisionShape3D.new()
    var box_shape: BoxShape3D = BoxShape3D.new()
    box_shape.size = case_mesh.size
    collision.shape = box_shape
    body.add_child(collision)

    var terminal_material: StandardMaterial3D = _material(Color(0.68, 0.70, 0.72, 1.0), 0.96, 0.12)
    _cylinder_child(body, "PositiveTerminal", 0.13, 0.16, Vector3(-0.28, 0.40, 0.0), terminal_material)
    _cylinder_child(body, "NegativeTerminal", 0.13, 0.16, Vector3(0.28, 0.40, 0.0), terminal_material)
    _cylinder_child(body, "PositiveCap", 0.085, 0.04, Vector3(-0.28, 0.49, 0.0),
        _material(Color(0.82, 0.12, 0.08, 1.0), 0.18, 0.32))
    _cylinder_child(body, "NegativeCap", 0.085, 0.04, Vector3(0.28, 0.49, 0.0),
        _material(Color(0.035, 0.04, 0.045, 1.0), 0.25, 0.42))
    _label3d_child(body, "PWR", Vector3(-0.34, 0.36, 0.59), 0.010, WHITE)

func _build_led() -> void:
    var body: StaticBody3D = StaticBody3D.new()
    body.name = "LedBody3D"
    body.set_meta("part_id", "led")
    body.position = Vector3(2.35, 0.53, 0.0)
    add_child(body)

    led_off_material = _material(Color(0.10, 0.34, 0.08, 0.96), 0.02, 0.18)
    led_on_material = _material(Color(0.34, 1.0, 0.20, 1.0), 0.0, 0.08, NEON, 5.0)

    led_mesh = MeshInstance3D.new()
    led_mesh.name = "LedDomeMesh3D"
    var sphere_mesh: SphereMesh = SphereMesh.new()
    sphere_mesh.radius = 0.31
    sphere_mesh.height = 0.56
    sphere_mesh.radial_segments = 32
    sphere_mesh.rings = 16
    led_mesh.mesh = sphere_mesh
    led_mesh.scale = Vector3(1.0, 1.16, 1.0)
    led_mesh.position = Vector3(0.0, 0.22, 0.0)
    led_mesh.material_override = led_off_material
    body.add_child(led_mesh)

    var collar: MeshInstance3D = _cylinder_child(body, "LedCollarMesh3D", 0.34, 0.16,
        Vector3(0.0, -0.08, 0.0), _material(Color(0.06, 0.16, 0.055, 1.0), 0.04, 0.28))
    collar.scale = Vector3(1.0, 1.0, 1.0)

    var collision: CollisionShape3D = CollisionShape3D.new()
    var sphere_shape: SphereShape3D = SphereShape3D.new()
    sphere_shape.radius = 0.38
    collision.position = Vector3(0.0, 0.18, 0.0)
    collision.shape = sphere_shape
    body.add_child(collision)

    var leg_material: StandardMaterial3D = _material(Color(0.72, 0.74, 0.75, 1.0), 0.94, 0.14)
    _box_child(body, "AnodeLeg", Vector3(0.08, 0.34, 0.08), Vector3(-0.12, -0.28, 0.0), leg_material)
    _box_child(body, "CathodeLeg", Vector3(0.08, 0.34, 0.08), Vector3(0.12, -0.28, 0.0), leg_material)
    _label3d_child(body, "LED", Vector3(-0.34, 0.68, 0.0), 0.010, WHITE)

    var glow_light: OmniLight3D = OmniLight3D.new()
    glow_light.name = "LedGlowLight3D"
    glow_light.position = Vector3(0.0, 0.34, 0.0)
    glow_light.light_color = NEON
    glow_light.light_energy = 0.0
    glow_light.omni_range = 3.3
    body.add_child(glow_light)

func _build_copper_path() -> void:
    var copper_material: StandardMaterial3D = _material(COPPER, 0.88, 0.22)
    _box("CopperTrace_A", Vector3(1.25, 0.035, 0.12), Vector3(-1.45, 0.175, 0.0), copper_material)
    _box("CopperTrace_B", Vector3(1.25, 0.035, 0.12), Vector3(1.45, 0.175, 0.0), copper_material)
    _box("CopperTrace_Center", Vector3(1.75, 0.035, 0.12), Vector3(0.0, 0.175, 0.0), copper_material)

    var wire_node: MeshInstance3D = _segment("Wire3D", pulse_start, pulse_end, 0.055,
        _material(Color(0.95, 0.30, 0.04, 1.0), 0.46, 0.30))
    wire_node.visible = false

    current_pulse = MeshInstance3D.new()
    current_pulse.name = "CurrentPulseMesh3D"
    var pulse_mesh: SphereMesh = SphereMesh.new()
    pulse_mesh.radius = 0.095
    pulse_mesh.height = 0.19
    current_pulse.mesh = pulse_mesh
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
    camera.look_at(Vector3(0.0, 0.25, 0.0), Vector3.UP)

func _build_hud() -> void:
    var layer: CanvasLayer = CanvasLayer.new()
    layer.name = "HUD"
    add_child(layer)

    var panel: ColorRect = ColorRect.new()
    panel.name = "InstructionPanel"
    panel.position = Vector2(18.0, 18.0)
    panel.size = Vector2(430.0, 194.0)
    panel.color = Color(0.015, 0.025, 0.030, 0.93)
    layer.add_child(panel)

    var title: Label = Label.new()
    title.position = Vector2(20.0, 16.0)
    title.text = "FORMFACTOR 1.06 // MATERIAL 3D LAB"
    title.add_theme_color_override("font_color", NEON)
    title.add_theme_font_size_override("font_size", 20)
    panel.add_child(title)

    step_label = Label.new()
    step_label.position = Vector2(20.0, 53.0)
    step_label.size = Vector2(390.0, 54.0)
    step_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    step_label.add_theme_color_override("font_color", WHITE)
    step_label.add_theme_font_size_override("font_size", 16)
    panel.add_child(step_label)

    status_label = Label.new()
    status_label.position = Vector2(20.0, 112.0)
    status_label.size = Vector2(390.0, 60.0)
    status_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    status_label.add_theme_color_override("font_color", Color(0.66, 0.76, 0.78, 1.0))
    status_label.add_theme_font_size_override("font_size", 14)
    panel.add_child(status_label)

    var controls: Label = Label.new()
    controls.position = Vector2(18.0, 830.0)
    controls.text = "LEFT CLICK PARTS  •  RIGHT/MIDDLE DRAG ORBIT  •  WHEEL ZOOM  •  R CAMERA  •  C RESET"
    controls.add_theme_color_override("font_color", Color(0.70, 0.78, 0.80, 1.0))
    controls.add_theme_font_size_override("font_size", 14)
    layer.add_child(controls)

    var truth_label: Label = Label.new()
    truth_label.position = Vector2(1010.0, 18.0)
    truth_label.size = Vector2(440.0, 78.0)
    truth_label.text = "3D DISPLAY LAYER\nMaterials and animation show state only.\nEngineering truth remains in the validated C++ core."
    truth_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
    truth_label.add_theme_color_override("font_color", Color(0.62, 0.73, 0.75, 1.0))
    truth_label.add_theme_font_size_override("font_size", 13)
    layer.add_child(truth_label)

func _pick_part(screen_pos: Vector2) -> void:
    var ray_origin: Vector3 = camera.project_ray_origin(screen_pos)
    var ray_end: Vector3 = ray_origin + camera.project_ray_normal(screen_pos) * 100.0
    var query: PhysicsRayQueryParameters3D = PhysicsRayQueryParameters3D.create(ray_origin, ray_end)
    query.collide_with_areas = true
    query.collide_with_bodies = true
    var hit: Dictionary = get_world_3d().direct_space_state.intersect_ray(query)
    if hit.is_empty():
        status_label.text = "Click the physical power block or LED on the board."
        return
    var collider: Object = hit.get("collider") as Object
    if collider == null:
        return
    var part_id: String = str(collider.get_meta("part_id", ""))
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
        status_label.text = "The training interaction is complete. The LED now uses a real emissive 3D material."

func _complete_circuit() -> void:
    circuit_live = true
    _set_step(2)
    led_mesh.material_override = led_on_material
    var led_body: Node = get_node_or_null("LedBody3D")
    if led_body != null:
        var glow_light: OmniLight3D = led_body.get_node_or_null("LedGlowLight3D") as OmniLight3D
        if glow_light != null:
            glow_light.light_energy = 5.5
    var wire_node: MeshInstance3D = get_node_or_null("Wire3D") as MeshInstance3D
    if wire_node != null:
        wire_node.visible = true
    current_pulse.visible = true

func _reset_tutorial() -> void:
    circuit_live = false
    led_mesh.material_override = led_off_material
    var led_body: Node = get_node_or_null("LedBody3D")
    if led_body != null:
        var glow_light: OmniLight3D = led_body.get_node_or_null("LedGlowLight3D") as OmniLight3D
        if glow_light != null:
            glow_light.light_energy = 0.0
    var wire_node: MeshInstance3D = get_node_or_null("Wire3D") as MeshInstance3D
    if wire_node != null:
        wire_node.visible = false
    current_pulse.visible = false
    _set_step(0)

func _material(color: Color, metallic: float = 0.0, roughness: float = 0.5,
               emission_color: Color = Color(0.0, 0.0, 0.0, 1.0),
               emission_energy: float = 0.0) -> StandardMaterial3D:
    var material: StandardMaterial3D = StandardMaterial3D.new()
    material.albedo_color = color
    material.metallic = metallic
    material.roughness = roughness
    if emission_energy > 0.0:
        material.emission_enabled = true
        material.emission = emission_color
        material.emission_energy_multiplier = emission_energy
    return material

func _box(node_name: String, size: Vector3, position_value: Vector3,
          material: Material) -> MeshInstance3D:
    var node: MeshInstance3D = MeshInstance3D.new()
    node.name = node_name
    var box_mesh: BoxMesh = BoxMesh.new()
    box_mesh.size = size
    node.mesh = box_mesh
    node.position = position_value
    node.material_override = material
    add_child(node)
    return node

func _box_child(parent: Node3D, node_name: String, size: Vector3, position_value: Vector3,
                material: Material) -> MeshInstance3D:
    var node: MeshInstance3D = MeshInstance3D.new()
    node.name = node_name
    var box_mesh: BoxMesh = BoxMesh.new()
    box_mesh.size = size
    node.mesh = box_mesh
    node.position = position_value
    node.material_override = material
    parent.add_child(node)
    return node

func _cylinder(node_name: String, radius: float, height: float, position_value: Vector3,
               material: Material) -> MeshInstance3D:
    var node: MeshInstance3D = MeshInstance3D.new()
    node.name = node_name
    var cylinder_mesh: CylinderMesh = CylinderMesh.new()
    cylinder_mesh.top_radius = radius
    cylinder_mesh.bottom_radius = radius
    cylinder_mesh.height = height
    cylinder_mesh.radial_segments = 28
    node.mesh = cylinder_mesh
    node.position = position_value
    node.material_override = material
    add_child(node)
    return node

func _cylinder_child(parent: Node3D, node_name: String, radius: float, height: float,
                     position_value: Vector3, material: Material) -> MeshInstance3D:
    var node: MeshInstance3D = MeshInstance3D.new()
    node.name = node_name
    var cylinder_mesh: CylinderMesh = CylinderMesh.new()
    cylinder_mesh.top_radius = radius
    cylinder_mesh.bottom_radius = radius
    cylinder_mesh.height = height
    cylinder_mesh.radial_segments = 28
    node.mesh = cylinder_mesh
    node.position = position_value
    node.material_override = material
    parent.add_child(node)
    return node

func _segment(node_name: String, from_point: Vector3, to_point: Vector3, radius: float,
              material: Material) -> MeshInstance3D:
    var direction: Vector3 = to_point - from_point
    var node: MeshInstance3D = MeshInstance3D.new()
    node.name = node_name
    var cylinder_mesh: CylinderMesh = CylinderMesh.new()
    cylinder_mesh.top_radius = radius
    cylinder_mesh.bottom_radius = radius
    cylinder_mesh.height = direction.length()
    cylinder_mesh.radial_segments = 24
    node.mesh = cylinder_mesh
    node.material_override = material
    node.position = (from_point + to_point) * 0.5
    var y_axis: Vector3 = direction.normalized()
    var x_axis: Vector3 = y_axis.cross(Vector3.FORWARD)
    if x_axis.length() < 0.01:
        x_axis = y_axis.cross(Vector3.RIGHT)
    x_axis = x_axis.normalized()
    var z_axis: Vector3 = x_axis.cross(y_axis).normalized()
    node.basis = Basis(x_axis, y_axis, z_axis)
    add_child(node)
    return node

func _label3d(text_value: String, position_value: Vector3, pixel_size_value: float,
              color_value: Color) -> Label3D:
    var label: Label3D = Label3D.new()
    label.name = "SilkLabel3D"
    label.text = text_value
    label.position = position_value
    label.rotation_degrees = Vector3(-90.0, 0.0, 0.0)
    label.pixel_size = pixel_size_value
    label.modulate = color_value
    add_child(label)
    return label

func _label3d_child(parent: Node3D, text_value: String, position_value: Vector3,
                    pixel_size_value: float, color_value: Color) -> Label3D:
    var label: Label3D = Label3D.new()
    label.name = "%s_Label3D" % text_value
    label.text = text_value
    label.position = position_value
    label.pixel_size = pixel_size_value
    label.modulate = color_value
    parent.add_child(label)
    return label
