class_name EngineeringCoreBridge
extends RefCounted

# The Godot prototype is never allowed to invent engineering truth.
# Until a native C++ bridge is connected, validation must stay unknown.
func validate_board(_snapshot: Dictionary) -> Dictionary:
    return {
        "status": "unknown",
        "passed": false,
        "message": "Engineering core is not connected. Result is UNKNOWN, not PASS."
    }
