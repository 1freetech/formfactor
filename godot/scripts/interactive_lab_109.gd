extends "res://scripts/interactive_lab_108.gd"

const COMPONENT_LEXICON := {
    "resistor": {"term":"Resistor", "symbol":"R / Ω", "role":"Limits current and creates voltage drops.", "use":"LED protection, biasing, pull-up/pull-down networks, and signal conditioning.", "check":"Resistance is measured in ohms. Verify value and power rating before energizing."},
    "potentiometer": {"term":"Potentiometer", "symbol":"RV / Ω", "role":"A three-terminal variable resistor with an adjustable wiper.", "use":"Manual voltage dividers, calibration, gain controls, and adjustable thresholds.", "check":"Check total resistance, wiper position, and which terminals are being used."},
    "ceramic_cap": {"term":"Ceramic capacitor", "symbol":"C / F", "role":"Stores charge and reacts quickly to changing voltage.", "use":"Decoupling, noise filtering, timing, and high-frequency bypass near IC power pins.", "check":"Capacitance is measured in farads; also check voltage rating and dielectric type."},
    "electrolytic_cap": {"term":"Electrolytic capacitor", "symbol":"C / F", "role":"Provides relatively large capacitance in a compact polarized package.", "use":"Bulk power smoothing, low-frequency filtering, and energy storage on supply rails.", "check":"Polarity matters. Reverse voltage can damage the part; check capacitance and voltage rating."},
    "inductor": {"term":"Inductor", "symbol":"L / H", "role":"Stores energy in a magnetic field and resists rapid current change.", "use":"Power converters, filters, chokes, and energy-storage stages.", "check":"Inductance is measured in henries; also verify current rating and winding resistance."},
    "diode": {"term":"Standard diode", "symbol":"D", "role":"Primarily allows current in one direction and blocks it in the other.", "use":"Rectification, reverse-polarity protection, clamping, and flyback paths.", "check":"Identify anode/cathode orientation and verify forward voltage with diode-test mode."},
    "zener": {"term":"Zener diode", "symbol":"D / Vz", "role":"Operates in controlled reverse breakdown near a specified Zener voltage.", "use":"Voltage references, simple regulation, and over-voltage clamps.", "check":"Check polarity, Zener voltage, current limiting, and power dissipation."},
    "led": {"term":"Light-emitting diode", "symbol":"LED", "role":"Emits light when forward biased while behaving electrically like a diode.", "use":"Status indicators, displays, optical signaling, and debugging feedback.", "check":"Observe anode/cathode polarity and always limit current with an appropriate resistor or driver."},
    "npn": {"term":"NPN transistor", "symbol":"Q", "role":"A bipolar junction transistor controlled by base current.", "use":"Low-side switching, amplification, drivers, and simple logic stages.", "check":"Identify base, collector, and emitter; check gain assumptions and base-current limiting."},
    "pnp": {"term":"PNP transistor", "symbol":"Q", "role":"A bipolar transistor commonly used for high-side or complementary switching.", "use":"High-side switching, complementary amplifiers, and current-source arrangements.", "check":"Identify base, collector, and emitter; voltage polarity is opposite the common NPN convention."},
    "nmos": {"term":"N-channel MOSFET", "symbol":"Q", "role":"A voltage-controlled transistor with low gate current and potentially low on-resistance.", "use":"Efficient low-side switching, power conversion, motor control, and digital load control.", "check":"Identify gate/drain/source, confirm gate-drive voltage, RDS(on), current, voltage, and thermal limits."},
    "pmos": {"term":"P-channel MOSFET", "symbol":"Q", "role":"A voltage-controlled transistor often used for high-side switching.", "use":"Load switches, reverse-polarity protection, and high-side power control.", "check":"Identify gate/drain/source and confirm the required negative gate-to-source drive and ratings."},
    "logic": {"term":"Logic gate / inverter", "symbol":"U", "role":"Implements a digital Boolean function such as NOT, AND, OR, NAND, or NOR.", "use":"Digital decision making, signal cleanup, enable logic, and state-machine building blocks.", "check":"Verify logic family, supply voltage, input thresholds, output drive, and unused-input handling."},
    "opamp": {"term":"Operational amplifier", "symbol":"U / OP AMP", "role":"A high-gain differential amplifier intended to be controlled with feedback.", "use":"Analog amplification, filtering, buffering, sensing, and mathematical signal operations.", "check":"Check supply rails, input/output range, feedback network, bandwidth, and stability."},
    "comparator": {"term":"Comparator", "symbol":"U / CMP", "role":"Compares two analog voltages and changes output state when one crosses the other.", "use":"Threshold detection, zero crossing, battery monitoring, and waveform shaping.", "check":"Check input common-mode range, output type, hysteresis needs, and reference voltage."},
    "regulator": {"term":"Voltage regulator", "symbol":"U / REG", "role":"Maintains a controlled output voltage from a varying input or load.", "use":"Power rails, point-of-load supplies, battery systems, and sensitive electronics.", "check":"Verify input/output voltage, current, dropout or switching requirements, heat, and required capacitors."},
    "fuse": {"term":"Fuse", "symbol":"F", "role":"Sacrificial over-current protection that opens a circuit when current is excessive.", "use":"Input protection, branch-circuit protection, and fault containment.", "check":"Match current, voltage, interrupt rating, and fast/slow-blow behavior to the protected circuit."},
    "switch": {"term":"Switch", "symbol":"SW", "role":"Mechanically or electronically opens, closes, or redirects a circuit path.", "use":"Power control, mode selection, user input, and isolation.", "check":"Verify contact arrangement, current/voltage rating, and whether switch bounce matters."},
    "relay": {"term":"Relay", "symbol":"K", "role":"Uses an energized coil or equivalent actuator to operate isolated switch contacts.", "use":"Galvanic isolation, high-power load control, motor control, and signal routing.", "check":"Verify coil voltage/current, contact ratings, contact form, and flyback protection for DC coils."},
    "connector": {"term":"Connector / header", "symbol":"J", "role":"Provides a removable electrical and mechanical interface between circuits or cables.", "use":"Power entry, programming, sensors, daughterboards, and external I/O.", "check":"Confirm pinout, pitch, polarity/keying, current per pin, and mating connector."},
    "test_point": {"term":"Test point", "symbol":"TP", "role":"A deliberate exposed node for measurement, probing, or manufacturing test.", "use":"Oscilloscope probes, multimeter checks, automated test fixtures, and debugging.", "check":"Know what signal should be present and never probe beyond the instrument or circuit safety limits."},
    "sensor": {"term":"Sensor", "symbol":"S", "role":"Converts a physical condition into an electrical signal or digital reading.", "use":"Temperature, pressure, light, motion, current, voltage, and environmental measurement.", "check":"Verify supply, interface, range, calibration, units, and signal-conditioning requirements."},
    "buzzer": {"term":"Buzzer / output", "symbol":"BZ", "role":"Converts electrical drive into audible output.", "use":"Alerts, confirmation tones, alarms, and diagnostic feedback.", "check":"Check operating voltage, current, frequency/drive type, polarity if applicable, and sound rating."},
    "power": {"term":"DC power source", "symbol":"PWR / V", "role":"Supplies electrical energy and establishes a voltage difference for the circuit.", "use":"Bench supplies, batteries, adapters, and regulated DC rails.", "check":"Verify voltage, current limit, polarity, grounding, and expected load before connection."},
    "ground": {"term":"Ground / reference", "symbol":"GND", "role":"Defines the circuit reference potential and often provides a common return path.", "use":"Signal reference, power return, shielding strategy, and measurement reference.", "check":"Ground is not automatically earth. Confirm which ground domain and return path the design intends."}
}

var lexicon_panel: ColorRect
var lexicon_selector: OptionButton
var lexicon_label: Label
var active_component: StaticBody3D = null

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.08"):
                label.text = "FORMFACTOR 1.09 // INTERACTIVE 3D BUILD LAB"
    _label3d("1.09 // BUILD • WIRE • TEST • LEARN", Vector3(0.95, 0.19, -1.82), 0.0075, NEON)

func _build_gameplay_hud() -> void:
    super._build_gameplay_hud()
    var hud := get_node_or_null("HUD") as CanvasLayer
    if hud == null:
        return

    lexicon_panel = ColorRect.new()
    lexicon_panel.name = "ComponentLexiconPanel"
    lexicon_panel.position = Vector2(404, 670)
    lexicon_panel.size = Vector2(650, 210)
    lexicon_panel.color = Color(0.012, 0.021, 0.026, 0.95)
    hud.add_child(lexicon_panel)

    var title := Label.new()
    title.position = Vector2(14, 10)
    title.text = "COMPONENT LEXICON // ALL 25 FAMILIES"
    title.add_theme_color_override("font_color", NEON)
    title.add_theme_font_size_override("font_size", 16)
    lexicon_panel.add_child(title)

    lexicon_selector = OptionButton.new()
    lexicon_selector.name = "LexiconSelector"
    lexicon_selector.position = Vector2(14, 38)
    lexicon_selector.size = Vector2(270, 34)
    var item_index: int = 0
    for component in COMPONENTS:
        var component_id: String = str(component["id"])
        lexicon_selector.add_item(str(component["name"]))
        lexicon_selector.set_item_metadata(item_index, component_id)
        item_index += 1
    lexicon_selector.item_selected.connect(_on_lexicon_selected)
    lexicon_panel.add_child(lexicon_selector)

    var rotate_left := Button.new()
    rotate_left.text = "ROTATE -90° (Q)"
    rotate_left.position = Vector2(300, 38)
    rotate_left.size = Vector2(155, 34)
    rotate_left.pressed.connect(func() -> void: rotate_active_component(-90.0))
    lexicon_panel.add_child(rotate_left)

    var rotate_right := Button.new()
    rotate_right.text = "ROTATE +90° (E)"
    rotate_right.position = Vector2(465, 38)
    rotate_right.size = Vector2(168, 34)
    rotate_right.pressed.connect(func() -> void: rotate_active_component(90.0))
    lexicon_panel.add_child(rotate_right)

    lexicon_label = Label.new()
    lexicon_label.name = "LexiconText"
    lexicon_label.position = Vector2(14, 80)
    lexicon_label.size = Vector2(620, 118)
    lexicon_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    lexicon_label.add_theme_color_override("font_color", Color(0.80, 0.88, 0.90, 1.0))
    lexicon_label.add_theme_font_size_override("font_size", 12)
    lexicon_panel.add_child(lexicon_label)

    _show_lexicon("resistor")

func select_component(component_id: String) -> void:
    super.select_component(component_id)
    _show_lexicon(component_id)

func _on_lexicon_selected(index: int) -> void:
    if lexicon_selector == null or index < 0:
        return
    var component_id: String = str(lexicon_selector.get_item_metadata(index))
    _show_lexicon(component_id)

func _show_lexicon(component_id: String) -> void:
    if lexicon_label == null:
        return
    var entry: Dictionary = COMPONENT_LEXICON.get(component_id, {}) as Dictionary
    if entry.is_empty():
        lexicon_label.text = "No lexicon entry is available for this component."
        return
    lexicon_label.text = "%s  [%s]\n%s\nUSE: %s\nCHECK: %s" % [
        str(entry["term"]), str(entry["symbol"]), str(entry["role"]), str(entry["use"]), str(entry["check"])
    ]
    if lexicon_selector != null:
        for i in range(lexicon_selector.item_count):
            if str(lexicon_selector.get_item_metadata(i)) == component_id:
                lexicon_selector.select(i)
                break

func _handle_world_click(screen_pos: Vector2) -> void:
    if wire_mode or not selected_component_id.is_empty():
        super._handle_world_click(screen_pos)
        return
    var collider: Object = _ray_collider(screen_pos)
    if collider != null and bool(collider.get_meta("placed_component", false)):
        active_component = collider as StaticBody3D
        if active_component != null:
            var component_id: String = str(active_component.get_meta("component_id", ""))
            var refdes: String = str(active_component.get_meta("refdes", "part"))
            _show_lexicon(component_id)
            status_label.text = "Selected %s. Rotate it with Q/E or the Lexicon rotation buttons." % refdes
            if build_status != null:
                build_status.text = "ACTIVE BOARD PART: %s" % refdes
            return
    super._handle_world_click(screen_pos)

func _unhandled_input(event: InputEvent) -> void:
    if event is InputEventKey:
        var key := event as InputEventKey
        if key.pressed and not key.echo:
            if key.keycode == KEY_Q:
                rotate_active_component(-90.0)
                get_viewport().set_input_as_handled()
                return
            if key.keycode == KEY_E:
                rotate_active_component(90.0)
                get_viewport().set_input_as_handled()
                return
    super._unhandled_input(event)

func rotate_active_component(degrees: float) -> void:
    if active_component == null or not is_instance_valid(active_component):
        if build_status != null:
            build_status.text = "ROTATE: click a placed component first."
        return
    active_component.rotate_y(deg_to_rad(degrees))
    var refdes: String = str(active_component.get_meta("refdes", "part"))
    if build_status != null:
        build_status.text = "ROTATED %s by %.0f°" % [refdes, degrees]
    status_label.text = "%s rotated. Component position stays locked to the PCB grid." % refdes

func _remove_at_screen(screen_pos: Vector2) -> void:
    var previous_active: StaticBody3D = active_component
    super._remove_at_screen(screen_pos)
    if previous_active != null and (not is_instance_valid(previous_active) or previous_active.is_queued_for_deletion()):
        active_component = null

func _clear_player_board() -> void:
    active_component = null
    super._clear_player_board()

func debug_lexicon_count() -> int:
    return COMPONENT_LEXICON.size()

func debug_lexicon_complete() -> bool:
    for component in COMPONENTS:
        if not COMPONENT_LEXICON.has(str(component["id"])):
            return false
    return COMPONENT_LEXICON.size() == COMPONENTS.size()

func debug_lexicon_panel_ready() -> bool:
    return lexicon_panel != null and lexicon_selector != null and lexicon_label != null

func debug_select_first_placed() -> bool:
    if placed_components.is_empty():
        return false
    active_component = placed_components[0]
    return active_component != null

func debug_active_rotation_y() -> float:
    if active_component == null or not is_instance_valid(active_component):
        return 0.0
    return active_component.rotation.y
