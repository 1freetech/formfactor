class_name EngineeringBridge1127
extends RefCounted

# Serialization boundary between the Godot editor and the authoritative
# engineering core. The snapshot is evidence of what the player drew only.

const SNAPSHOT_FORMAT := "formfactor-editor-snapshot-v1"
const ALLOWED_PIN_TYPES := {
    "passive": true,
    "power_input": true,
    "power_output": true,
    "digital_input": true,
    "digital_output": true,
    "open_drain": true
}

static func _safe_token(value: String) -> bool:
    if value.is_empty():
        return false
    for code in value.to_ascii_buffer():
        var ok := (code >= 48 and code <= 57) or (code >= 65 and code <= 90) or (code >= 97 and code <= 122) or code == 95 or code == 45 or code == 46
        if not ok:
            return false
    return true

static func validate_pin_catalog(pin_library: Dictionary) -> Array[String]:
    var errors: Array[String] = []
    var component_ids: Array = pin_library.keys()
    component_ids.sort()
    for component_variant in component_ids:
        var component_id := str(component_variant)
        if not _safe_token(component_id):
            errors.append("unsafe component id: %s" % component_id)
            continue
        var pins_variant = pin_library[component_id]
        if pins_variant is not Array or (pins_variant as Array).is_empty():
            errors.append("component has no pins: %s" % component_id)
            continue
        var seen: Dictionary = {}
        for pin_variant in pins_variant:
            if pin_variant is not Dictionary:
                errors.append("invalid pin entry: %s" % component_id)
                continue
            var pin := pin_variant as Dictionary
            var pin_id := str(pin.get("id", ""))
            var pin_type := str(pin.get("type", ""))
            if not _safe_token(pin_id):
                errors.append("unsafe pin id: %s.%s" % [component_id, pin_id])
            elif seen.has(pin_id):
                errors.append("duplicate pin id: %s.%s" % [component_id, pin_id])
            else:
                seen[pin_id] = true
            if not ALLOWED_PIN_TYPES.has(pin_type):
                errors.append("unsupported pin type: %s.%s=%s" % [component_id, pin_id, pin_type])
    return errors

static func build_snapshot(
    placed_components: Array[StaticBody3D],
    user_wires: Array[Dictionary],
    pin_library: Dictionary,
    package_links: Dictionary,
    nets: Array[Dictionary]
) -> Dictionary:
    var components: Array[Dictionary] = []
    for body in placed_components:
        if body == null or not is_instance_valid(body):
            continue
        var component_id := str(body.get_meta("component_id", ""))
        components.append({
            "refdes": str(body.get_meta("refdes", "")),
            "component_id": component_id,
            "position": [body.position.x, body.position.y, body.position.z],
            "rotation_y": body.rotation.y,
            "pins": (pin_library.get(component_id, []) as Array).duplicate(true),
            "engineering_link": (package_links.get(component_id, {}) as Dictionary).duplicate(true)
        })
    components.sort_custom(func(a: Dictionary, b: Dictionary) -> bool:
        return str(a.get("refdes", "")) < str(b.get("refdes", ""))
    )

    var wires: Array[Dictionary] = []
    for wire in user_wires:
        var a := wire.get("a") as StaticBody3D
        var b := wire.get("b") as StaticBody3D
        if a == null or b == null or not is_instance_valid(a) or not is_instance_valid(b):
            continue
        wires.append({
            "a_ref": str(a.get_meta("refdes", "")),
            "a_pin": str(wire.get("a_pin", "")),
            "b_ref": str(b.get_meta("refdes", "")),
            "b_pin": str(wire.get("b_pin", ""))
        })
    wires.sort_custom(func(a: Dictionary, b: Dictionary) -> bool:
        var ak := "%s.%s>%s.%s" % [str(a.get("a_ref", "")), str(a.get("a_pin", "")), str(a.get("b_ref", "")), str(a.get("b_pin", ""))]
        var bk := "%s.%s>%s.%s" % [str(b.get("a_ref", "")), str(b.get("a_pin", "")), str(b.get("b_ref", "")), str(b.get("b_pin", ""))]
        return ak < bk
    )

    return {
        "format": SNAPSHOT_FORMAT,
        "editor_version": "1.127",
        "engineering_truth": "cpp_core_required",
        "components": components,
        "wires": wires,
        "nets": nets.duplicate(true)
    }

static func write_snapshot(snapshot: Dictionary, path := "user://last_engineering_board_1_127.json") -> bool:
    var file := FileAccess.open(path, FileAccess.WRITE)
    if file == null:
        return false
    file.store_string(JSON.stringify(snapshot, "  ", false))
    file.close()
    return true
