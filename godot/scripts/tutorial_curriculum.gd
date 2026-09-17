extends RefCounted
class_name FormFactorTutorialCurriculum

const CERTIFICATE_TIERS := [
    {"lessons":10, "grade":"Grade I", "title":"PCB Foundations Certificate"},
    {"lessons":25, "grade":"Grade II", "title":"Circuit Builder Certificate"},
    {"lessons":50, "grade":"Grade III", "title":"Electronics Technician Certificate"},
    {"lessons":75, "grade":"Grade IV", "title":"Advanced Board Technician Certificate"},
    {"lessons":100, "grade":"Grade V", "title":"Open Source Engineer Certificate"}
]

const MODULES := [
    {"name":"Board Basics", "parts":["resistor","power","ground","led","switch","test_point"], "titles":["Place Your First Resistor","Add Power and Ground","Light an LED","Current-Limited LED","Add a Switch","Build a Series Path","Build a Parallel Branch","Add a Test Point","Read the Schematic Mirror","Basic Indicator Board"]},
    {"name":"Passive Networks", "parts":["resistor","potentiometer","ceramic_cap","electrolytic_cap","inductor","power","ground"], "titles":["Two-Resistor Network","Voltage Divider","Adjustable Divider","Ceramic Decoupling","Bulk Capacitance","RC Filter","RL Filter","Mixed Passive Network","Power Rail Filtering","Passive Network Board"]},
    {"name":"Diodes and Protection", "parts":["diode","zener","led","fuse","power","ground","resistor"], "titles":["Diode Direction","Reverse Protection","Zener Clamp","LED Polarity","Fuse the Input","Protected LED Rail","Clamp a Signal","Dual-Diode Path","Protected Power Entry","Protection Board"]},
    {"name":"Transistor Switching", "parts":["npn","pnp","nmos","pmos","resistor","led","power","ground","switch"], "titles":["NPN Switch","PNP High Side","MOSFET Gate Basics","NMOS Load Switch","PMOS Load Switch","Base Resistor Choice","Transistor LED Driver","Complementary Pair","Manual Control Stage","Transistor Control Board"]},
    {"name":"Power Regulation", "parts":["regulator","fuse","electrolytic_cap","ceramic_cap","diode","power","ground","test_point"], "titles":["Regulator Placement","Input Protection","Input Capacitor","Output Capacitor","Protected Regulator","Power Test Points","Filtered Rail","Regulated Output","Power Distribution","Regulated Supply Board"]},
    {"name":"Digital Logic", "parts":["logic","switch","led","resistor","power","ground","connector","test_point"], "titles":["Logic Power Pins","Inverter Input","Logic Output LED","Manual Logic Input","Two Logic Stages","Connector Input","Logic Test Points","Signal Indicator","Small Control Chain","Digital Control Board"]},
    {"name":"Analog and Sensors", "parts":["opamp","comparator","sensor","potentiometer","resistor","ceramic_cap","power","ground","led"], "titles":["Sensor Power","Comparator Threshold","Potentiometer Reference","Op-Amp Placement","Sensor Indicator","Filtered Sensor","Comparator LED","Analog Reference","Mixed Analog Chain","Sensor Interface Board"]},
    {"name":"Interfaces and Outputs", "parts":["connector","relay","buzzer","switch","test_point","diode","nmos","power","ground"], "titles":["Header Layout","Buzzer Output","Relay Coil","Flyback Diode","MOSFET Relay Driver","External Switch Input","Output Connector","Test Header","Protected Output Stage","I/O Control Board"]},
    {"name":"Troubleshooting", "parts":["test_point","power","ground","resistor","led","diode","fuse","regulator","logic","sensor"], "titles":["Find an Open Path","Find a Missing Ground","Find a Reversed Diode","Find an Unpowered LED","Trace a Blown Fuse Path","Check a Regulated Rail","Trace a Logic Signal","Trace a Sensor Signal","Repair a Mixed Circuit","Diagnostic Board Challenge"]},
    {"name":"Capstone Boards", "parts":["power","ground","fuse","regulator","connector","switch","sensor","logic","nmos","relay","buzzer","led","test_point","resistor","ceramic_cap","electrolytic_cap"], "titles":["Protected Indicator Controller","Sensor Alarm Board","Regulated Logic Board","MOSFET Output Controller","Relay Interface Board","Power and Signal Board","Multi-Stage Control Board","Serviceable Test Board","Integrated Systems Board","Final PCB Systems Capstone"]}
]

static func difficulty_for_lesson(lesson_number: int) -> float:
    return pow(1.0125, float(max(0, lesson_number - 1)))

static func lessons() -> Array:
    var output: Array = []
    var lesson_number := 1
    for module_index in range(MODULES.size()):
        var module: Dictionary = MODULES[module_index]
        var titles: Array = module["titles"]
        var part_pool: Array = module["parts"]
        for local_index in range(10):
            var required_parts: Array[String] = []
            var desired_count: int = mini(part_pool.size(), 1 + int(local_index / 2))
            if module_index >= 4:
                desired_count = mini(part_pool.size(), desired_count + 1)
            if module_index >= 8:
                desired_count = mini(part_pool.size(), desired_count + 1)
            for part_index in range(desired_count):
                required_parts.append(str(part_pool[part_index]))
            var minimum_wires: int = maxi(0, desired_count - 1)
            if lesson_number >= 25:
                minimum_wires += 1
            if lesson_number >= 50:
                minimum_wires += 1
            if lesson_number >= 75:
                minimum_wires += 1
            var hint_count: int = 4
            if lesson_number > 25:
                hint_count = 3
            if lesson_number > 50:
                hint_count = 2
            if lesson_number > 75:
                hint_count = 1
            if lesson_number == 100:
                hint_count = 0
            output.append({
                "number": lesson_number,
                "module": str(module["name"]),
                "title": str(titles[local_index]),
                "difficulty": difficulty_for_lesson(lesson_number),
                "required_parts": required_parts,
                "minimum_wires": minimum_wires,
                "hints": hint_count,
                "objective": _objective_text(lesson_number, str(module["name"]), str(titles[local_index]), required_parts, minimum_wires)
            })
            lesson_number += 1
    return output

static func certificate_for_completed(completed_lessons: int) -> Dictionary:
    var earned: Dictionary = {}
    for tier in CERTIFICATE_TIERS:
        if completed_lessons >= int(tier["lessons"]):
            earned = tier.duplicate(true)
    return earned

static func next_certificate_for_completed(completed_lessons: int) -> Dictionary:
    for tier in CERTIFICATE_TIERS:
        if completed_lessons < int(tier["lessons"]):
            return tier.duplicate(true)
    return {}

static func _objective_text(lesson_number: int, module_name: String, title: String, required_parts: Array[String], minimum_wires: int) -> String:
    var part_names := ", ".join(required_parts)
    var cumulative_percent := (difficulty_for_lesson(lesson_number) - 1.0) * 100.0
    return "Lesson %d — %s / %s. Build on the PCB using: %s. Make at least %d wire connection(s). Each lesson is 1.25%% harder than the previous lesson; cumulative difficulty is +%.1f%% versus Lesson 1. Use the live schematic mirror and TEST before completion." % [lesson_number, module_name, title, part_names, minimum_wires, cumulative_percent]
