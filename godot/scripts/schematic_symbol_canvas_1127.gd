extends "res://scripts/schematic_symbol_canvas_1126.gd"

# 1.127 displays the exact pin endpoints selected by the player. This remains
# presentation-only; it mirrors the editor snapshot and never certifies a net.

const PIN_TEXT := Color(0.98, 0.67, 0.20, 0.94)

func _draw_connections(row_y: Dictionary) -> void:
    var lane := 0
    var font := ThemeDB.fallback_font
    for connection_variant in connection_rows:
        var connection := connection_variant as Dictionary
        var a_ref := str(connection.get("a_ref", connection.get("a", "")))
        var b_ref := str(connection.get("b_ref", connection.get("b", "")))
        if not row_y.has(a_ref) or not row_y.has(b_ref):
            continue

        var a_pin := str(connection.get("a_pin", "?"))
        var b_pin := str(connection.get("b_pin", "?"))
        var y_a := float(row_y[a_ref])
        var y_b := float(row_y[b_ref])
        var lane_x := 286.0 + float(lane % 5) * 12.0
        lane += 1
        var start := Vector2(274.0, y_a)
        var finish := Vector2(274.0, y_b)

        draw_circle(start, 2.8, CONNECTION)
        draw_circle(finish, 2.8, CONNECTION)
        draw_line(start, Vector2(lane_x, y_a), CONNECTION, 1.7, true)
        draw_line(Vector2(lane_x, y_a), Vector2(lane_x, y_b), CONNECTION, 1.7, true)
        draw_line(Vector2(lane_x, y_b), finish, CONNECTION, 1.7, true)

        if lane <= 6:
            var label := "%s.%s ↔ %s.%s" % [a_ref, a_pin, b_ref, b_pin]
            var label_y := minf(y_a, y_b) + 12.0 + float((lane - 1) % 2) * 10.0
            draw_string(font, Vector2(12, label_y), label, HORIZONTAL_ALIGNMENT_LEFT, 150, 8, PIN_TEXT)

func debug_pin_labels_present() -> bool:
    for connection_variant in connection_rows:
        var connection := connection_variant as Dictionary
        if str(connection.get("a_pin", "")).is_empty() or str(connection.get("b_pin", "")).is_empty():
            return false
    return true
