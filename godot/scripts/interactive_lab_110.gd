extends "res://scripts/interactive_lab_109.gd"

const CURRICULUM = preload("res://scripts/tutorial_curriculum.gd")
const PROGRESS_PATH := "user://formfactor_tutorial_progress.cfg"
const CERTIFICATE_DIR := "user://certificates"

var dragging_component: StaticBody3D = null
var dragging_original_position := Vector3.ZERO
var tutorial_lessons: Array = []
var completed_lessons: Dictionary = {}
var tutorial_panel: ColorRect
var tutorial_selector: OptionButton
var tutorial_title: Label
var tutorial_objective: Label
var tutorial_requirements: Label
var tutorial_progress: Label
var certificate_status: Label
var learner_name: LineEdit
var learner_email: LineEdit
var current_lesson_index := 0
var certificate_http: HTTPRequest

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.09"):
                label.text = "FORMFACTOR 1.10 // PCB TRAINING + BUILD LAB"
    _label3d("1.10 // DRAG • ROTATE • LEARN • CERTIFY", Vector3(0.45, 0.19, -1.57), 0.0072, NEON)

func _build_gameplay_hud() -> void:
    super._build_gameplay_hud()
    tutorial_lessons = CURRICULUM.lessons()
    _load_tutorial_progress()
    var hud := get_node_or_null("HUD") as CanvasLayer
    if hud == null:
        return

    var learn_button := Button.new()
    learn_button.name = "OpenTutorialButton"
    learn_button.text = "LEARN // 100 LESSONS"
    learn_button.position = Vector2(404, 112)
    learn_button.size = Vector2(190, 40)
    learn_button.pressed.connect(_toggle_tutorial_panel)
    hud.add_child(learn_button)

    tutorial_panel = ColorRect.new()
    tutorial_panel.name = "TutorialPanel"
    tutorial_panel.position = Vector2(404, 158)
    tutorial_panel.size = Vector2(650, 500)
    tutorial_panel.color = Color(0.008, 0.018, 0.024, 0.985)
    tutorial_panel.visible = false
    hud.add_child(tutorial_panel)

    var header := Label.new()
    header.position = Vector2(16, 12)
    header.text = "FORMFACTOR TRAINING PATH // 100 PCB LESSONS"
    header.add_theme_color_override("font_color", NEON)
    header.add_theme_font_size_override("font_size", 18)
    tutorial_panel.add_child(header)

    var close_button := Button.new()
    close_button.text = "CLOSE"
    close_button.position = Vector2(562, 8)
    close_button.size = Vector2(72, 34)
    close_button.pressed.connect(_toggle_tutorial_panel)
    tutorial_panel.add_child(close_button)

    tutorial_selector = OptionButton.new()
    tutorial_selector.name = "TutorialSelector"
    tutorial_selector.position = Vector2(16, 52)
    tutorial_selector.size = Vector2(618, 36)
    for lesson_variant in tutorial_lessons:
        var lesson: Dictionary = lesson_variant
        tutorial_selector.add_item("%03d  %s — %s" % [int(lesson["number"]), str(lesson["module"]), str(lesson["title"])])
    tutorial_selector.item_selected.connect(_on_tutorial_selected)
    tutorial_panel.add_child(tutorial_selector)

    tutorial_progress = Label.new()
    tutorial_progress.position = Vector2(16, 98)
    tutorial_progress.size = Vector2(618, 28)
    tutorial_progress.add_theme_color_override("font_color", Color(0.78, 0.88, 0.90, 1.0))
    tutorial_panel.add_child(tutorial_progress)

    tutorial_title = Label.new()
    tutorial_title.position = Vector2(16, 130)
    tutorial_title.size = Vector2(618, 32)
    tutorial_title.add_theme_color_override("font_color", WHITE)
    tutorial_title.add_theme_font_size_override("font_size", 17)
    tutorial_panel.add_child(tutorial_title)

    tutorial_objective = Label.new()
    tutorial_objective.position = Vector2(16, 166)
    tutorial_objective.size = Vector2(618, 86)
    tutorial_objective.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    tutorial_objective.add_theme_color_override("font_color", Color(0.82, 0.88, 0.90, 1.0))
    tutorial_objective.add_theme_font_size_override("font_size", 13)
    tutorial_panel.add_child(tutorial_objective)

    tutorial_requirements = Label.new()
    tutorial_requirements.position = Vector2(16, 258)
    tutorial_requirements.size = Vector2(618, 60)
    tutorial_requirements.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    tutorial_requirements.add_theme_color_override("font_color", NEON)
    tutorial_requirements.add_theme_font_size_override("font_size", 13)
    tutorial_panel.add_child(tutorial_requirements)

    var quick_guide := Label.new()
    quick_guide.position = Vector2(16, 322)
    quick_guide.size = Vector2(618, 48)
    quick_guide.text = "PLAY: choose a part → click PCB to place → drag placed part to move → Q/E rotates it → WIRE connects parts → TEST checks the circuit. Drag empty space to rotate the camera."
    quick_guide.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    quick_guide.add_theme_color_override("font_color", Color(0.68, 0.78, 0.80, 1.0))
    quick_guide.add_theme_font_size_override("font_size", 12)
    tutorial_panel.add_child(quick_guide)

    learner_name = LineEdit.new()
    learner_name.name = "CertificateName"
    learner_name.position = Vector2(16, 378)
    learner_name.size = Vector2(220, 34)
    learner_name.placeholder_text = "Name for certificates"
    learner_name.text = str(_progress_value("learner_name", ""))
    learner_name.text_changed.connect(func(_value: String) -> void: _save_tutorial_progress())
    tutorial_panel.add_child(learner_name)

    learner_email = LineEdit.new()
    learner_email.name = "CertificateEmail"
    learner_email.position = Vector2(244, 378)
    learner_email.size = Vector2(260, 34)
    learner_email.placeholder_text = "Email for certificates"
    learner_email.text = str(_progress_value("learner_email", ""))
    learner_email.text_changed.connect(func(_value: String) -> void: _save_tutorial_progress())
    tutorial_panel.add_child(learner_email)

    var check_button := Button.new()
    check_button.name = "CheckLessonButton"
    check_button.text = "CHECK LESSON"
    check_button.position = Vector2(512, 378)
    check_button.size = Vector2(122, 34)
    check_button.pressed.connect(_check_current_lesson)
    tutorial_panel.add_child(check_button)

    certificate_status = Label.new()
    certificate_status.position = Vector2(16, 420)
    certificate_status.size = Vector2(618, 64)
    certificate_status.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
    certificate_status.add_theme_color_override("font_color", Color(0.82, 0.88, 0.90, 1.0))
    certificate_status.add_theme_font_size_override("font_size", 12)
    tutorial_panel.add_child(certificate_status)

    certificate_http = HTTPRequest.new()
    certificate_http.name = "CertificateEmailRequest"
    certificate_http.request_completed.connect(_on_certificate_request_completed)
    add_child(certificate_http)

    _show_tutorial(0)

func _unhandled_input(event: InputEvent) -> void:
    if event is InputEventMouseButton:
        var mb := event as InputEventMouseButton
        if mb.button_index == MOUSE_BUTTON_LEFT:
            if mb.pressed and selected_component_id.is_empty() and not wire_mode:
                var collider := _ray_collider(mb.position)
                if collider != null and bool(collider.get_meta("placed_component", false)):
                    dragging_component = collider as StaticBody3D
                    if dragging_component != null:
                        active_component = dragging_component
                        dragging_original_position = dragging_component.position
                        _show_lexicon(str(dragging_component.get_meta("component_id", "")))
                        status_label.text = "Moving %s. Drag it to another open PCB grid position; Q/E rotates it." % str(dragging_component.get_meta("refdes", "part"))
                        get_viewport().set_input_as_handled()
                        return
            elif not mb.pressed and dragging_component != null:
                var refdes := str(dragging_component.get_meta("refdes", "part"))
                status_label.text = "%s moved to its new PCB position. Q/E rotates the selected part." % refdes
                dragging_component = null
                _refresh_schematic()
                get_viewport().set_input_as_handled()
                return
    elif event is InputEventMouseMotion and dragging_component != null:
        var motion := event as InputEventMouseMotion
        var target := _screen_to_board(motion.position)
        if target != Vector3.INF:
            var blocker := _component_near_position_except(target, dragging_component)
            if blocker == null:
                dragging_component.position = target
                _refresh_wires_for_component(dragging_component)
                _reset_player_test_visuals()
                if build_status != null:
                    build_status.text = "MOVE: %s → grid %.1f, %.1f" % [str(dragging_component.get_meta("refdes", "part")), target.x, target.z]
            elif build_status != null:
                build_status.text = "MOVE BLOCKED: %s occupies that area." % str(blocker.get_meta("refdes", "part"))
        get_viewport().set_input_as_handled()
        return
    super._unhandled_input(event)

func _component_near_position_except(world_pos: Vector3, ignored: StaticBody3D) -> StaticBody3D:
    var target := Vector2(world_pos.x, world_pos.z)
    for body in placed_components:
        if body == null or not is_instance_valid(body) or body == ignored:
            continue
        var existing := Vector2(body.position.x, body.position.z)
        if existing.distance_to(target) < PLACEMENT_CLEARANCE:
            return body
    return null

func _refresh_wires_for_component(body: StaticBody3D) -> void:
    for wire in user_wires:
        var a := wire.get("a") as StaticBody3D
        var b := wire.get("b") as StaticBody3D
        if a != body and b != body:
            continue
        var old_node := wire.get("node") as Node
        if old_node != null and is_instance_valid(old_node):
            old_node.queue_free()
        if a == null or b == null or not is_instance_valid(a) or not is_instance_valid(b):
            continue
        var start := a.position + Vector3(0.0, 0.56, 0.0)
        var finish := b.position + Vector3(0.0, 0.56, 0.0)
        var new_node := _segment("UserWire3D", start, finish, 0.045, _material(NORMAL_WIRE_COLOR, 0.52, 0.28))
        wire["node"] = new_node

func _toggle_tutorial_panel() -> void:
    if tutorial_panel == null:
        return
    tutorial_panel.visible = not tutorial_panel.visible

func _on_tutorial_selected(index: int) -> void:
    _show_tutorial(index)

func _show_tutorial(index: int) -> void:
    if tutorial_lessons.is_empty():
        return
    current_lesson_index = clampi(index, 0, tutorial_lessons.size() - 1)
    var lesson: Dictionary = tutorial_lessons[current_lesson_index]
    if tutorial_selector != null:
        tutorial_selector.select(current_lesson_index)
    if tutorial_title != null:
        tutorial_title.text = "LESSON %d // %s" % [int(lesson["number"]), str(lesson["title"])]
    if tutorial_objective != null:
        tutorial_objective.text = str(lesson["objective"])
    if tutorial_requirements != null:
        tutorial_requirements.text = "REQUIRED PARTS: %s\nMINIMUM WIRES: %d • HINTS AVAILABLE: %d • DIFFICULTY: %.0f%%" % [", ".join(lesson["required_parts"]), int(lesson["minimum_wires"]), int(lesson["hints"]), float(lesson["difficulty"]) * 100.0]
    _refresh_tutorial_progress()

func _check_current_lesson() -> void:
    if tutorial_lessons.is_empty():
        return
    var lesson: Dictionary = tutorial_lessons[current_lesson_index]
    var missing: Array[String] = []
    for required_id_variant in lesson["required_parts"]:
        var required_id := str(required_id_variant)
        if _count_placed_component(required_id) <= 0:
            missing.append(required_id)
    if not missing.is_empty():
        certificate_status.text = "LESSON NOT COMPLETE — missing component families: %s" % ", ".join(missing)
        return
    if user_wires.size() < int(lesson["minimum_wires"]):
        certificate_status.text = "LESSON NOT COMPLETE — add at least %d wire connection(s). Current: %d." % [int(lesson["minimum_wires"]), user_wires.size()]
        return
    var required_parts: Array = lesson["required_parts"]
    if required_parts.has("power") and required_parts.has("led"):
        var connected := _find_connected_power_led_path()
        if connected.is_empty():
            certificate_status.text = "LESSON NOT COMPLETE — the power source must have a continuous connected path to the LED."
            return
    var lesson_number := int(lesson["number"])
    completed_lessons[lesson_number] = true
    _save_tutorial_progress()
    certificate_status.text = "LESSON %d COMPLETE — %s" % [lesson_number, str(lesson["title"])]
    _handle_certificate_milestone()
    _refresh_tutorial_progress()

func _count_placed_component(component_id: String) -> int:
    var count := 0
    for body in placed_components:
        if body != null and is_instance_valid(body) and str(body.get_meta("component_id", "")) == component_id:
            count += 1
    return count

func _refresh_tutorial_progress() -> void:
    if tutorial_progress == null:
        return
    var completed_count := completed_lessons.size()
    var lesson_number := current_lesson_index + 1
    var lesson_state := "COMPLETE" if completed_lessons.has(lesson_number) else "IN PROGRESS"
    var next_tier: Dictionary = CURRICULUM.next_certificate_for_completed(completed_count)
    var next_text := "All certificate tiers earned."
    if not next_tier.is_empty():
        next_text = "Next certificate: %s at %d lessons" % [str(next_tier["grade"]), int(next_tier["lessons"])]
    tutorial_progress.text = "PROGRESS: %d / 100 lessons • Selected: %s • %s" % [completed_count, lesson_state, next_text]
    var earned: Dictionary = CURRICULUM.certificate_for_completed(completed_count)
    if certificate_status != null and not earned.is_empty() and certificate_status.text.is_empty():
        certificate_status.text = "HIGHEST EARNED: %s — %s" % [str(earned["grade"]), str(earned["title"])]

func _handle_certificate_milestone() -> void:
    var completed_count := completed_lessons.size()
    var tier: Dictionary = CURRICULUM.certificate_for_completed(completed_count)
    if tier.is_empty() or int(tier["lessons"]) != completed_count:
        return
    var name_text := learner_name.text.strip_edges() if learner_name != null else ""
    var email_text := learner_email.text.strip_edges() if learner_email != null else ""
    if name_text.is_empty():
        certificate_status.text = "%s earned. Enter your name to generate the digital certificate." % str(tier["grade"])
        return
    var certificate_path := _write_certificate_html(name_text, tier)
    if email_text.is_empty():
        certificate_status.text = "%s earned. Certificate saved locally. Add an email address for delivery when the secure mail service is configured." % str(tier["grade"])
        return
    _request_certificate_email(name_text, email_text, tier, certificate_path)

func _write_certificate_html(name_text: String, tier: Dictionary) -> String:
    DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path(CERTIFICATE_DIR))
    var filename := "FormFactor-%s-%03d-lessons.html" % [str(tier["grade"]).replace(" ", "-"), int(tier["lessons"])]
    var path := CERTIFICATE_DIR + "/" + filename
    var file := FileAccess.open(path, FileAccess.WRITE)
    if file == null:
        return ""
    var html := "<!doctype html><html><head><meta charset='utf-8'><title>FormFactor Certificate</title></head><body style='background:#050807;color:#dfffe0;font-family:monospace;text-align:center;padding:70px'><div style='border:3px solid #39ff14;padding:60px'><h1 style='color:#39ff14'>FORMFACTOR</h1><h2>%s — %s</h2><p>This digital training completion certificate is awarded to</p><h1>%s</h1><p>for completing %d of 100 progressive PCB construction and troubleshooting lessons.</p><p>Each lesson increases in designed difficulty by 1%% from Lesson 1 through Lesson 100.</p><p>FormFactor training completion certificate • not a third-party professional accreditation</p></div></body></html>" % [str(tier["grade"]), str(tier["title"]), name_text.xml_escape(), int(tier["lessons"])]
    file.store_string(html)
    file.close()
    return path

func _request_certificate_email(name_text: String, email_text: String, tier: Dictionary, certificate_path: String) -> void:
    var endpoint := OS.get_environment("FORMFACTOR_CERTIFICATE_API").strip_edges()
    if endpoint.is_empty():
        certificate_status.text = "%s earned. Certificate saved locally. Automatic email is ready for a secure FORMFACTOR_CERTIFICATE_API backend, but no mail endpoint is configured in this build." % str(tier["grade"])
        return
    if certificate_http == null:
        return
    var payload := JSON.stringify({"name":name_text, "email":email_text, "grade":str(tier["grade"]), "title":str(tier["title"]), "lessons":int(tier["lessons"]), "certificate_path":certificate_path})
    var error := certificate_http.request(endpoint, ["Content-Type: application/json"], HTTPClient.METHOD_POST, payload)
    if error != OK:
        certificate_status.text = "Certificate generated, but the email request could not start. Error %d." % error
    else:
        certificate_status.text = "Certificate generated. Secure email delivery request sent."

func _on_certificate_request_completed(_result: int, response_code: int, _headers: PackedStringArray, _body: PackedByteArray) -> void:
    if certificate_status == null:
        return
    if response_code >= 200 and response_code < 300:
        certificate_status.text = "Certificate email accepted by the configured delivery service."
    else:
        certificate_status.text = "Certificate is saved locally, but email delivery returned HTTP %d." % response_code

func _load_tutorial_progress() -> void:
    completed_lessons.clear()
    var config := ConfigFile.new()
    if config.load(PROGRESS_PATH) != OK:
        return
    var stored: Array = config.get_value("tutorial", "completed_lessons", [])
    for value in stored:
        completed_lessons[int(value)] = true

func _save_tutorial_progress() -> void:
    var config := ConfigFile.new()
    var stored: Array[int] = []
    for lesson_number in completed_lessons.keys():
        stored.append(int(lesson_number))
    stored.sort()
    config.set_value("tutorial", "completed_lessons", stored)
    if learner_name != null:
        config.set_value("profile", "learner_name", learner_name.text)
    if learner_email != null:
        config.set_value("profile", "learner_email", learner_email.text)
    config.save(PROGRESS_PATH)

func _progress_value(key: String, default_value: Variant) -> Variant:
    var config := ConfigFile.new()
    if config.load(PROGRESS_PATH) != OK:
        return default_value
    return config.get_value("profile", key, default_value)

func debug_tutorial_count() -> int:
    return tutorial_lessons.size()

func debug_certificate_tier_count() -> int:
    return CURRICULUM.CERTIFICATE_TIERS.size()

func debug_difficulty_for_lesson(lesson_number: int) -> float:
    return CURRICULUM.difficulty_for_lesson(lesson_number)

func debug_move_component(body: StaticBody3D, target: Vector3) -> bool:
    if body == null or not is_instance_valid(body):
        return false
    var blocker := _component_near_position_except(target, body)
    if blocker != null:
        return false
    body.position = target
    _refresh_wires_for_component(body)
    return true

func debug_tutorial_panel_ready() -> bool:
    return tutorial_panel != null and tutorial_selector != null and tutorial_title != null and tutorial_objective != null
