extends "res://scripts/interactive_lab.gd"

const PLACEMENT_CLEARANCE := 0.72
const TEST_WIRE_COLOR := Color(0.35, 1.0, 0.18, 1.0)
const NORMAL_WIRE_COLOR := Color(0.95, 0.34, 0.06, 1.0)
const LED_OFF_COLOR := Color(0.86, 0.10, 0.08, 1.0)

var last_player_test_passed := false
var last_player_test_path_length := 0

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.07"):
                label.text = "FORMFACTOR 1.08 // INTERACTIVE 3D BUILD LAB"
    _label3d("1.08 // CONNECTED CIRCUIT TEST", Vector3(1.45, 0.19, -1.82), 0.0075, NEON)

func _place_component_at_world(world_pos: Vector3) -> StaticBody3D:
    var blocker := _component_near_position(world_pos)
    if blocker != null:
        var blocker_ref := str(blocker.get_meta("refdes", "part"))
        status_label.text = "That PCB spot is occupied by %s. Pick another grid position." % blocker_ref
        if build_status != null:
            build_status.text = "PLACE BLOCKED: %s is too close." % blocker_ref
        return null
    return super._place_component_at_world(world_pos)

func _component_near_position(world_pos: Vector3) -> StaticBody3D:
    var target := Vector2(world_pos.x, world_pos.z)
    for body in placed_components:
        if body == null or not is_instance_valid(body):
            continue
        var existing := Vector2(body.position.x, body.position.z)
        if existing.distance_to(target) < PLACEMENT_CLEARANCE:
            return body
    return null

func _test_player_circuit() -> void:
    last_player_test_passed = false
    last_player_test_path_length = 0
    _reset_player_test_visuals()

    var power_count := 0
    var led_count := 0
    for body in placed_components:
        if body == null or not is_instance_valid(body):
            continue
        var component_id := str(body.get_meta("component_id", ""))
        if component_id == "power":
            power_count += 1
        elif component_id == "led":
            led_count += 1

    if power_count == 0 or led_count == 0:
        if build_status != null:
            build_status.text = "TEST FAIL: add a DC power source and LED."
        status_label.text = "Test failed: the player board needs at least one power source and one LED."
        return

    if user_wires.is_empty():
        if build_status != null:
            build_status.text = "TEST FAIL: no wires on the player board."
        status_label.text = "Test failed: use WIRE and create a real path from power to an LED."
        return

    var result := _find_connected_power_led_path()
    if result.is_empty():
        if build_status != null:
            build_status.text = "TEST FAIL: wires do not connect PWR to LED."
        status_label.text = "Test failed: a stray wire is not a circuit. Connect the power source through a continuous wire path to an LED."
        return

    var led_body := result.get("led") as StaticBody3D
    var path_wires: Array = result.get("wires", [])
    _light_player_led(led_body)
    for wire in path_wires:
        var wire_node := wire.get("node") as MeshInstance3D
        if wire_node != null and is_instance_valid(wire_node):
            wire_node.material_override = _material(TEST_WIRE_COLOR, 0.18, 0.14, TEST_WIRE_COLOR, 3.0)

    last_player_test_passed = true
    last_player_test_path_length = path_wires.size()
    if build_status != null:
        build_status.text = "TEST PASS: connected path uses %d wire(s)." % path_wires.size()
    status_label.text = "Connected player circuit found. The placed LED and its actual power path are now lit."

func _find_connected_power_led_path() -> Dictionary:
    var bodies_by_id: Dictionary = {}
    var adjacency: Dictionary = {}
    var power_ids: Array[int] = []
    var led_ids: Dictionary = {}

    for body in placed_components:
        if body == null or not is_instance_valid(body):
            continue
        var object_id := body.get_instance_id()
        bodies_by_id[object_id] = body
        adjacency[object_id] = []
        var component_id := str(body.get_meta("component_id", ""))
        if component_id == "power":
            power_ids.append(object_id)
        elif component_id == "led":
            led_ids[object_id] = true

    for wire in user_wires:
        var a := wire.get("a") as StaticBody3D
        var b := wire.get("b") as StaticBody3D
        if a == null or b == null or not is_instance_valid(a) or not is_instance_valid(b):
            continue
        var a_id := a.get_instance_id()
        var b_id := b.get_instance_id()
        if not adjacency.has(a_id) or not adjacency.has(b_id):
            continue
        adjacency[a_id].append({"neighbor": b_id, "wire": wire})
        adjacency[b_id].append({"neighbor": a_id, "wire": wire})

    var queue: Array[int] = []
    var visited: Dictionary = {}
    var previous_node: Dictionary = {}
    var previous_wire: Dictionary = {}

    for power_id in power_ids:
        queue.append(power_id)
        visited[power_id] = true

    while not queue.is_empty():
        var current: int = int(queue.pop_front())
        if led_ids.has(current):
            var path_wires: Array = []
            var cursor: int = current
            while previous_node.has(cursor):
                path_wires.push_front(previous_wire[cursor])
                cursor = int(previous_node[cursor])
            return {"led": bodies_by_id[current], "wires": path_wires}

        var edges: Array = adjacency.get(current, [])
        for edge in edges:
            var neighbor := int(edge.get("neighbor", 0))
            if visited.has(neighbor):
                continue
            visited[neighbor] = true
            previous_node[neighbor] = current
            previous_wire[neighbor] = edge.get("wire")
            queue.append(neighbor)

    return {}

func _reset_player_test_visuals() -> void:
    for body in placed_components:
        if body == null or not is_instance_valid(body):
            continue
        if str(body.get_meta("component_id", "")) == "led":
            var meshes := body.find_children("*", "MeshInstance3D", true, false)
            for child in meshes:
                var mesh := child as MeshInstance3D
                if mesh != null:
                    mesh.material_override = _material(LED_OFF_COLOR, 0.02, 0.18)
            var glow := body.get_node_or_null("PlayerLedGlow") as OmniLight3D
            if glow != null:
                glow.light_energy = 0.0

    for wire in user_wires:
        var wire_node := wire.get("node") as MeshInstance3D
        if wire_node != null and is_instance_valid(wire_node):
            wire_node.material_override = _material(NORMAL_WIRE_COLOR, 0.52, 0.28)

func _light_player_led(led_body: StaticBody3D) -> void:
    if led_body == null or not is_instance_valid(led_body):
        return
    var meshes := led_body.find_children("*", "MeshInstance3D", true, false)
    for child in meshes:
        var mesh := child as MeshInstance3D
        if mesh != null:
            mesh.material_override = _material(Color(0.60, 1.0, 0.30, 1.0), 0.0, 0.08, NEON, 6.0)

    var glow := led_body.get_node_or_null("PlayerLedGlow") as OmniLight3D
    if glow == null:
        glow = OmniLight3D.new()
        glow.name = "PlayerLedGlow"
        glow.position = Vector3(0.0, 0.48, 0.0)
        glow.light_color = NEON
        glow.omni_range = 2.6
        led_body.add_child(glow)
    glow.light_energy = 5.0

func _remove_at_screen(screen_pos: Vector2) -> void:
    last_player_test_passed = false
    last_player_test_path_length = 0
    super._remove_at_screen(screen_pos)
    _reset_player_test_visuals()

func _clear_player_board() -> void:
    last_player_test_passed = false
    last_player_test_path_length = 0
    super._clear_player_board()

func debug_position_available(world_pos: Vector3) -> bool:
    return _component_near_position(world_pos) == null

func debug_last_test_passed() -> bool:
    return last_player_test_passed

func debug_last_test_path_length() -> int:
    return last_player_test_path_length

func debug_player_led_lit() -> bool:
    for body in placed_components:
        if body == null or not is_instance_valid(body):
            continue
        if str(body.get_meta("component_id", "")) != "led":
            continue
        var glow := body.get_node_or_null("PlayerLedGlow") as OmniLight3D
        if glow != null and glow.light_energy > 0.0:
            return true
    return false
