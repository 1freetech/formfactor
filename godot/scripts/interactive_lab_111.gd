extends "res://scripts/interactive_lab_110_125.gd"

const CAD_SCALE := 0.03
const CAD_OUTPUT_DIR := "user://cad"

var cad_panel: ColorRect
var cad_prompt: LineEdit
var cad_status: Label
var cad_outliner: ItemList
var cad_source_preview: TextEdit
var cad_width: SpinBox
var cad_depth: SpinBox
var cad_thickness: SpinBox
var cad_hole_diameter: SpinBox
var cad_hole_inset: SpinBox
var cad_holes_enabled: CheckButton
var cad_preview_root: CSGCombiner3D
var cad_viewport: SubViewport

func _retitle_existing_lab() -> void:
    super._retitle_existing_lab()
    var hud := get_node_or_null("HUD")
    if hud != null:
        for child in hud.find_children("*", "Label", true, false):
            var label := child as Label
            if label != null and label.text.begins_with("FORMFACTOR 1.10"):
                label.text = "FORMFACTOR 1.11 // PCB + PARAMETRIC CAD LAB"
    _label3d("1.11 // BUILD • CAD • EXPORT", Vector3(-0.55, 0.19, -1.57), 0.0072, Color(0.22, 1.0, 0.08, 1.0))

func _build_gameplay_hud() -> void:
    super._build_gameplay_hud()
    var hud := get_node_or_null("HUD") as CanvasLayer
    if hud == null:
        return

    var open_button := Button.new()
    open_button.name = "OpenCadWorkbenchButton"
    open_button.text = "CAD // PARAMETRIC"
    open_button.position = Vector2(604, 112)
    open_button.size = Vector2(190, 40)
    open_button.pressed.connect(_toggle_cad_panel)
    hud.add_child(open_button)

    cad_panel = ColorRect.new()
    cad_panel.name = "CadWorkbenchPanel"
    cad_panel.position = Vector2(404, 158)
    cad_panel.size = Vector2(650, 530)
    cad_panel.color = Color(0.006, 0.014, 0.020, 0.992)
    cad_panel.visible = false
    hud.add_child(cad_panel)

    var header := Label.new()
    header.position = Vector2(16, 10)
    header.text = "CAD WORKBENCH // PARAMETRIC PLATE + 4-HOLE FIXTURE"
    header.add_theme_color_override("font_color", Color(0.22, 1.0, 0.08, 1.0))
    header.add_theme_font_size_override("font_size", 17)
    cad_panel.add_child(header)

    var close_button := Button.new()
    close_button.text = "CLOSE"
    close_button.position = Vector2(562, 8)
    close_button.size = Vector2(72, 34)
    close_button.pressed.connect(_toggle_cad_panel)
    cad_panel.add_child(close_button)

    cad_prompt = LineEdit.new()
    cad_prompt.name = "CadPrompt"
    cad_prompt.position = Vector2(16, 48)
    cad_prompt.size = Vector2(468, 36)
    cad_prompt.placeholder_text = "Example: 120x80x2 mm plate, 4 holes diameter 3.2mm, inset 5mm"
    cad_prompt.text = "100x70x2 mm plate, 4 holes diameter 3.2mm, inset 5mm"
    cad_panel.add_child(cad_prompt)

    var prompt_button := Button.new()
    prompt_button.name = "ApplyCadPromptButton"
    prompt_button.text = "APPLY TEXT"
    prompt_button.position = Vector2(492, 48)
    prompt_button.size = Vector2(142, 36)
    prompt_button.pressed.connect(_apply_cad_prompt)
    cad_panel.add_child(prompt_button)

    var tree_label := Label.new()
    tree_label.position = Vector2(16, 94)
    tree_label.text = "SCENE / FEATURE TREE"
    tree_label.add_theme_color_override("font_color", Color(0.78, 0.88, 0.90, 1.0))
    tree_label.add_theme_font_size_override("font_size", 12)
    cad_panel.add_child(tree_label)

    cad_outliner = ItemList.new()
    cad_outliner.name = "CadFeatureTree"
    cad_outliner.position = Vector2(16, 116)
    cad_outliner.size = Vector2(190, 112)
    cad_outliner.item_selected.connect(_on_cad_feature_selected)
    cad_panel.add_child(cad_outliner)

    var inspector_label := Label.new()
    inspector_label.position = Vector2(16, 236)
    inspector_label.text = "PARAMETER INSPECTOR // millimeters"
    inspector_label.add_theme_color_override("font_color", Color(0.78, 0.88, 0.90, 1.0))
    inspector_label.add_theme_font_size_override("font_size", 12)
    cad_panel.add_child(inspector_label)

    cad_width = _cad_spin("WIDTH", Vector2(16, 262), 100.0, 10.0, 1000.0, 1.0)
    cad_depth = _cad_spin("DEPTH", Vector2(16, 300), 70.0, 10.0, 1000.0, 1.0)
    cad_thickness = _cad_spin("THICK", Vector2(16, 338), 2.0, 0.2, 100.0, 0.1)
    cad_hole_diameter = _cad_spin("HOLE Ø", Vector2(16, 376), 3.2, 0.5, 100.0, 0.1)
    cad_hole_inset = _cad_spin("INSET", Vector2(16, 414), 5.0, 0.5, 200.0, 0.5)

    cad_holes_enabled = CheckButton.new()
    cad_holes_enabled.name = "CadFourHoles"
    cad_holes_enabled.text = "4 MOUNTING HOLES"
    cad_holes_enabled.position = Vector2(16, 452)
    cad_holes_enabled.size = Vector2(190, 32)
    cad_holes_enabled.button_pressed = true
    cad_holes_enabled.toggled.connect(func(_pressed: bool) -> void: _update_cad_model())
    cad_panel.add_child(cad_holes_enabled)

    _build_cad_viewport()

    var source_label := Label.new()
    source_label.position = Vector2(220, 324)
    source_label.text = "DETERMINISTIC OPENSCAD SOURCE"
    source_label.add_theme_color_override("font_color", Color(0.78, 0.88, 0.90, 1.0))
    source_label.add_theme_font_size_override("font_size", 12)
    cad_panel.add_child(source_label)

    cad_source_preview = TextEdit.new()
    cad_source_preview.name = "CadSourcePreview"
    cad_source_preview.position = Vector2(220, 346)
    cad_source_preview.size = Vector2(414, 112)
    cad_source_preview.editable = false
    cad_source_preview.wrap_mode = TextEdit.LINE_WRAPPING_NONE
    cad_source_preview.add_theme_font_size_override("font_size", 11)
    cad_panel.add_child(cad_source_preview)

    var save_button := Button.new()
    save_button.text = "SAVE .SCAD"
    save_button.position = Vector2(220, 466)
    save_button.size = Vector2(126, 34)
    save_button.pressed.connect(func() -> void: _save_cad_source(true))
    cad_panel.add_child(save_button)

    var stl_button := Button.new()
    stl_button.text = "EXPORT STL"
    stl_button.position = Vector2(354, 466)
    stl_button.size = Vector2(126, 34)
    stl_button.pressed.connect(_export_cad_stl)
    cad_panel.add_child(stl_button)

    var step_button := Button.new()
    step_button.text = "EXPORT STEP"
    step_button.position = Vector2(488, 466)
    step_button.size = Vector2(146, 34)
    step_button.pressed.connect(_export_cad_step)
    cad_panel.add_child(step_button)

    cad_status = Label.new()
    cad_status.name = "CadStatus"
    cad_status.position = Vector2(16, 500)
    cad_status.size = Vector2(618, 24)
    cad_status.add_theme_color_override("font_color", Color(0.68, 0.82, 0.84, 1.0))
    cad_status.add_theme_font_size_override("font_size", 11)
    cad_panel.add_child(cad_status)

    _update_cad_model()

func _cad_spin(label_text: String, pos: Vector2, initial: float, minimum: float, maximum: float, step: float) -> SpinBox:
    var label := Label.new()
    label.position = pos
    label.size = Vector2(70, 32)
    label.text = label_text
    label.add_theme_color_override("font_color", Color(0.72, 0.80, 0.82, 1.0))
    label.add_theme_font_size_override("font_size", 11)
    cad_panel.add_child(label)

    var spin := SpinBox.new()
    spin.position = pos + Vector2(72, -4)
    spin.size = Vector2(118, 34)
    spin.min_value = minimum
    spin.max_value = maximum
    spin.step = step
    spin.value = initial
    spin.value_changed.connect(func(_value: float) -> void: _update_cad_model())
    cad_panel.add_child(spin)
    return spin

func _build_cad_viewport() -> void:
    var viewport_box := SubViewportContainer.new()
    viewport_box.name = "CadViewportContainer"
    viewport_box.position = Vector2(220, 94)
    viewport_box.size = Vector2(414, 220)
    viewport_box.stretch = true
    cad_panel.add_child(viewport_box)

    cad_viewport = SubViewport.new()
    cad_viewport.name = "CadViewport"
    cad_viewport.size = Vector2i(828, 440)
    cad_viewport.own_world_3d = true
    cad_viewport.render_target_update_mode = SubViewport.UPDATE_ALWAYS
    cad_viewport.transparent_bg = false
    viewport_box.add_child(cad_viewport)

    var world_environment := WorldEnvironment.new()
    var environment := Environment.new()
    environment.background_mode = Environment.BG_COLOR
    environment.background_color = Color(0.018, 0.026, 0.030, 1.0)
    environment.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
    environment.ambient_light_color = Color(0.65, 0.72, 0.76, 1.0)
    environment.ambient_light_energy = 0.48
    world_environment.environment = environment
    cad_viewport.add_child(world_environment)

    var light := DirectionalLight3D.new()
    light.rotation_degrees = Vector3(-52.0, -35.0, 0.0)
    light.light_energy = 1.35
    cad_viewport.add_child(light)

    var camera := Camera3D.new()
    camera.name = "CadCamera"
    camera.position = Vector3(0.0, 4.6, 6.8)
    cad_viewport.add_child(camera)
    camera.look_at(Vector3.ZERO, Vector3.UP)
    camera.current = true

    cad_preview_root = CSGCombiner3D.new()
    cad_preview_root.name = "CadPreviewRoot"
    cad_viewport.add_child(cad_preview_root)

func _toggle_cad_panel() -> void:
    if cad_panel == null:
        return
    cad_panel.visible = not cad_panel.visible
    if cad_panel.visible and tutorial_panel != null:
        tutorial_panel.visible = false

func _apply_cad_prompt() -> void:
    if cad_prompt == null:
        return
    var text := cad_prompt.text.to_lower().replace("×", "x")
    var dims := RegEx.new()
    dims.compile("([0-9]+(?:\\.[0-9]+)?)\\s*x\\s*([0-9]+(?:\\.[0-9]+)?)(?:\\s*x\\s*([0-9]+(?:\\.[0-9]+)?))?")
    var dim_match := dims.search(text)
    if dim_match != null:
        cad_width.value = float(dim_match.get_string(1))
        cad_depth.value = float(dim_match.get_string(2))
        if not dim_match.get_string(3).is_empty():
            cad_thickness.value = float(dim_match.get_string(3))

    var hole_regex := RegEx.new()
    hole_regex.compile("holes?[^0-9]*([0-9]+(?:\\.[0-9]+)?)")
    var hole_match := hole_regex.search(text)
    if hole_match != null:
        cad_hole_diameter.value = float(hole_match.get_string(1))

    var inset_regex := RegEx.new()
    inset_regex.compile("inset[^0-9]*([0-9]+(?:\\.[0-9]+)?)")
    var inset_match := inset_regex.search(text)
    if inset_match != null:
        cad_hole_inset.value = float(inset_match.get_string(1))

    if text.contains("no holes") or text.contains("without holes") or text.contains("solid plate"):
        cad_holes_enabled.button_pressed = false
    elif text.contains("4 holes") or text.contains("four holes") or text.contains("4 mounting holes"):
        cad_holes_enabled.button_pressed = true

    _update_cad_model()
    if _cad_model_valid():
        cad_status.text = "TEXT → PARAMETRIC MODEL: prompt applied and preview rebuilt."

func _cad_model_valid() -> bool:
    if cad_width == null or cad_depth == null or cad_thickness == null:
        return false
    var width := float(cad_width.value)
    var depth := float(cad_depth.value)
    var thickness := float(cad_thickness.value)
    var hole_d := float(cad_hole_diameter.value)
    var inset := float(cad_hole_inset.value)
    if width <= 0.0 or depth <= 0.0 or thickness <= 0.0:
        if cad_status != null:
            cad_status.text = "CAD BLOCKED: width, depth, and thickness must be greater than zero."
        return false
    if cad_holes_enabled != null and cad_holes_enabled.button_pressed:
        var radius := hole_d * 0.5
        if hole_d <= 0.0 or inset <= radius:
            if cad_status != null:
                cad_status.text = "CAD BLOCKED: hole inset must be larger than the hole radius."
            return false
        if inset + radius >= minf(width, depth) * 0.5:
            if cad_status != null:
                cad_status.text = "CAD BLOCKED: mounting holes do not fit inside the plate."
            return false
    return true

func _update_cad_model() -> void:
    if cad_panel == null:
        return
    if not _cad_model_valid():
        return
    _rebuild_cad_preview()
    _refresh_cad_outliner()
    if cad_source_preview != null:
        cad_source_preview.text = _build_openscad_source()
    if cad_status != null:
        cad_status.text = "PARAMETRIC MODEL READY // edit dimensions, apply text, or export."

func _refresh_cad_outliner() -> void:
    if cad_outliner == null:
        return
    cad_outliner.clear()
    cad_outliner.add_item("▾ Model: FormFactor Plate")
    cad_outliner.add_item("  ├─ Base plate")
    if cad_holes_enabled.button_pressed:
        for i in range(4):
            var branch := "  ├─" if i < 3 else "  └─"
            cad_outliner.add_item("%s Hole %d (subtract)" % [branch, i + 1])
    else:
        cad_outliner.add_item("  └─ Solid / no holes")
    cad_outliner.select(0)

func _on_cad_feature_selected(index: int) -> void:
    if cad_status == null or cad_outliner == null or index < 0:
        return
    cad_status.text = "FEATURE SELECTED: %s // scene tree + inspector stay synchronized." % cad_outliner.get_item_text(index)

func _rebuild_cad_preview() -> void:
    if cad_preview_root == null:
        return
    for child in cad_preview_root.get_children():
        child.queue_free()

    var material := StandardMaterial3D.new()
    material.albedo_color = Color(0.12, 0.55, 0.21, 1.0)
    material.metallic = 0.22
    material.roughness = 0.32

    var plate := CSGBox3D.new()
    plate.name = "Plate"
    plate.size = Vector3(cad_width.value * CAD_SCALE, cad_thickness.value * CAD_SCALE, cad_depth.value * CAD_SCALE)
    plate.material = material
    cad_preview_root.add_child(plate)

    if cad_holes_enabled.button_pressed:
        var xs := [-cad_width.value * 0.5 + cad_hole_inset.value, cad_width.value * 0.5 - cad_hole_inset.value]
        var zs := [-cad_depth.value * 0.5 + cad_hole_inset.value, cad_depth.value * 0.5 - cad_hole_inset.value]
        var hole_index := 1
        for x_value in xs:
            for z_value in zs:
                var hole := CSGCylinder3D.new()
                hole.name = "Hole%d" % hole_index
                hole.radius = cad_hole_diameter.value * 0.5 * CAD_SCALE
                hole.height = cad_thickness.value * CAD_SCALE + 0.20
                hole.sides = 40
                hole.operation = CSGShape3D.OPERATION_SUBTRACTION
                hole.position = Vector3(x_value * CAD_SCALE, 0.0, z_value * CAD_SCALE)
                cad_preview_root.add_child(hole)
                hole_index += 1

func _cad_hole_centers() -> Array[Vector2]:
    if cad_holes_enabled == null or not cad_holes_enabled.button_pressed:
        return []
    return [
        Vector2(cad_hole_inset.value, cad_hole_inset.value),
        Vector2(cad_width.value - cad_hole_inset.value, cad_hole_inset.value),
        Vector2(cad_width.value - cad_hole_inset.value, cad_depth.value - cad_hole_inset.value),
        Vector2(cad_hole_inset.value, cad_depth.value - cad_hole_inset.value)
    ]

func _build_openscad_source() -> String:
    if not _cad_model_valid():
        return "// Invalid FormFactor CAD model; export blocked.\n"
    var lines: Array[String] = [
        "// FormFactor deterministic CAD export",
        "// Units: millimeters",
        "width = %.4f;" % cad_width.value,
        "depth = %.4f;" % cad_depth.value,
        "thickness = %.4f;" % cad_thickness.value,
        "hole_d = %.4f;" % cad_hole_diameter.value,
        "hole_inset = %.4f;" % cad_hole_inset.value,
        "",
        "difference() {",
        "  cube([width, depth, thickness], center=false);"
    ]
    for center in _cad_hole_centers():
        lines.append("  translate([%.4f, %.4f, -0.5000]) cylinder(h=thickness + 1.0000, d=hole_d, $fn=48);" % [center.x, center.y])
    lines.append("}")
    lines.append("")
    return "\n".join(lines)

func _save_cad_source(show_status: bool = true) -> String:
    if not _cad_model_valid():
        return ""
    DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path(CAD_OUTPUT_DIR))
    var path := CAD_OUTPUT_DIR + "/formfactor_plate.scad"
    var file := FileAccess.open(path, FileAccess.WRITE)
    if file == null:
        if cad_status != null:
            cad_status.text = "CAD SAVE FAILED: could not open the user CAD folder."
        return ""
    file.store_string(_build_openscad_source())
    file.close()
    if show_status and cad_status != null:
        cad_status.text = "SAVED: %s" % ProjectSettings.globalize_path(path)
    return path

func _export_cad_stl() -> void:
    var source_path := _save_cad_source(false)
    if source_path.is_empty():
        return
    var global_source := ProjectSettings.globalize_path(source_path)
    var stl_path := CAD_OUTPUT_DIR + "/formfactor_plate.stl"
    var global_stl := ProjectSettings.globalize_path(stl_path)
    var output: Array = []
    var exit_code := OS.execute("openscad", PackedStringArray(["-o", global_stl, global_source]), output, true)
    if exit_code == 0 and FileAccess.file_exists(stl_path):
        cad_status.text = "OPENSCAD STL READY: %s" % global_stl
    else:
        cad_status.text = "OPENSCAD NOT AVAILABLE OR EXPORT FAILED. .SCAD source was still saved."

func _build_freecad_macro(step_path: String) -> String:
    var centers: Array[String] = []
    for center in _cad_hole_centers():
        centers.append("(%.4f, %.4f)" % [center.x, center.y])
    var quoted_path := JSON.stringify(step_path)
    var lines: Array[String] = [
        "# FormFactor deterministic FreeCAD export",
        "import FreeCAD as App",
        "import Part",
        "",
        "width = %.4f" % cad_width.value,
        "depth = %.4f" % cad_depth.value,
        "thickness = %.4f" % cad_thickness.value,
        "hole_d = %.4f" % cad_hole_diameter.value,
        "hole_centers = [%s]" % ", ".join(centers),
        "",
        "doc = App.newDocument('FormFactorCAD')",
        "shape = Part.makeBox(width, depth, thickness)",
        "for x, y in hole_centers:",
        "    tool = Part.makeCylinder(hole_d / 2.0, thickness + 1.0, App.Vector(x, y, -0.5))",
        "    shape = shape.cut(tool)",
        "obj = doc.addObject('PartDesign::Feature', 'FormFactorPlate')",
        "obj.Label = 'FormFactor Plate'",
        "obj.Shape = shape",
        "doc.recompute()",
        "Part.export([obj], %s)" % quoted_path,
        ""
    ]
    return "\n".join(lines)

func _export_cad_step() -> void:
    if not _cad_model_valid():
        return
    DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path(CAD_OUTPUT_DIR))
    var macro_path := CAD_OUTPUT_DIR + "/formfactor_plate.freecad.py"
    var step_path := CAD_OUTPUT_DIR + "/formfactor_plate.step"
    var global_macro := ProjectSettings.globalize_path(macro_path)
    var global_step := ProjectSettings.globalize_path(step_path)
    var file := FileAccess.open(macro_path, FileAccess.WRITE)
    if file == null:
        cad_status.text = "FREECAD EXPORT FAILED: could not create the generated macro."
        return
    file.store_string(_build_freecad_macro(global_step))
    file.close()

    var candidates := ["FreeCADCmd", "freecadcmd", "FreeCADCmd.exe", "freecadcmd.exe"]
    for executable in candidates:
        var output: Array = []
        var exit_code := OS.execute(executable, PackedStringArray([global_macro]), output, true)
        if exit_code == 0 and FileAccess.file_exists(step_path):
            cad_status.text = "FREECAD STEP READY: %s" % global_step
            return
    cad_status.text = "FREECADCMD NOT AVAILABLE OR EXPORT FAILED. Generated macro was saved."

func debug_cad_panel_ready() -> bool:
    return cad_panel != null and cad_prompt != null and cad_outliner != null and cad_source_preview != null and cad_preview_root != null

func debug_cad_dimensions() -> Vector3:
    if cad_width == null or cad_depth == null or cad_thickness == null:
        return Vector3.ZERO
    return Vector3(cad_width.value, cad_depth.value, cad_thickness.value)

func debug_cad_hole_count() -> int:
    if cad_holes_enabled == null or not cad_holes_enabled.button_pressed:
        return 0
    return 4

func debug_cad_source_contains_csg() -> bool:
    if cad_source_preview == null:
        return false
    return cad_source_preview.text.contains("difference()") and cad_source_preview.text.contains("cylinder(")

func debug_apply_cad_prompt(text: String) -> bool:
    if cad_prompt == null:
        return false
    cad_prompt.text = text
    _apply_cad_prompt()
    return _cad_model_valid()

func debug_cad_preview_shape_count() -> int:
    if cad_preview_root == null:
        return 0
    return cad_preview_root.get_child_count()
