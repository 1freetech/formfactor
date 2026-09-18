extends "res://scripts/schematic_symbol_canvas_1125.gd"

# FormFactor 1.126
# Adds real connection routing to the vector schematic mirror. The lines are a
# presentation of player-created links only; they do not claim electrical
# validity and never replace the authoritative engineering core.

const CONNECTION := Color(1.0, 0.55, 0.08, 0.88)
const CONNECTION_MUTED := Color(0.38, 0.50, 0.52, 0.72)

var connection_rows: Array = []

func set_circuit_graph(rows: Array, connections: Array) -> void:
    component_rows = rows.duplicate(true)
    connection_rows = connections.duplicate(true)
    wire_count = connection_rows.size()
    queue_redraw()

func _draw() -> void:
    draw_rect(Rect2(Vector2.ZERO, size), BG)
    var font := ThemeDB.fallback_font
    if component_rows.is_empty():
        draw_string(font, Vector2(12, 24), "SCHEMATIC", HORIZONTAL_ALIGNMENT_LEFT, -1, 12, NEON)
        draw_string(font, Vector2(12, 50), "Place a part to see its standard symbol.", HORIZONTAL_ALIGNMENT_LEFT, -1, 11, MUTED)
        draw_string(font, Vector2(12, size.y - 12), "WIRES  0", HORIZONTAL_ALIGNMENT_LEFT, -1, 10, MUTED)
        return

    var visible_count := mini(component_rows.size(), 7)
    var row_y: Dictionary = {}
    for i in range(visible_count):
        var row: Dictionary = component_rows[i]
        var y := 27.0 + float(i) * 45.0
        row_y[str(row.get("refdes", "?"))] = y
        _draw_symbol(str(row.get("id", "")), Vector2(18.0, y))
        draw_string(font, Vector2(120, y + 4), str(row.get("refdes", "?")), HORIZONTAL_ALIGNMENT_LEFT, 44, 11, NEON)
        draw_string(font, Vector2(166, y + 4), str(row.get("name", "")), HORIZONTAL_ALIGNMENT_LEFT, 110, 10, FG)

    _draw_connections(row_y)

    if component_rows.size() > visible_count:
        draw_string(font, Vector2(12, size.y - 27), "+ %d more parts" % (component_rows.size() - visible_count), HORIZONTAL_ALIGNMENT_LEFT, -1, 10, MUTED)
    draw_string(font, Vector2(240, size.y - 11), "WIRES  %d" % wire_count, HORIZONTAL_ALIGNMENT_LEFT, -1, 10, MUTED)

func _draw_connections(row_y: Dictionary) -> void:
    var lane := 0
    for connection_variant in connection_rows:
        var connection := connection_variant as Dictionary
        var a_ref := str(connection.get("a", ""))
        var b_ref := str(connection.get("b", ""))
        if not row_y.has(a_ref) or not row_y.has(b_ref):
            continue
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

func debug_connection_count() -> int:
    return connection_rows.size()
