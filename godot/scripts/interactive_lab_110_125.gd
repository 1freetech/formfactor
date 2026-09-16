extends "res://scripts/interactive_lab_110.gd"

const CERTIFICATE_OUTPUT_DIR := "user://certificates"
const ACADEMY_CURRICULUM = preload("res://scripts/tutorial_curriculum.gd")

func _write_certificate_html(name_text: String, tier: Dictionary) -> String:
    DirAccess.make_dir_recursive_absolute(ProjectSettings.globalize_path(CERTIFICATE_OUTPUT_DIR))
    var filename := "FormFactor-%s-%03d-lessons.html" % [str(tier["grade"]).replace(" ", "-"), int(tier["lessons"])]
    var path := CERTIFICATE_OUTPUT_DIR + "/" + filename
    var file := FileAccess.open(path, FileAccess.WRITE)
    if file == null:
        return ""
    var html := "<!doctype html><html><head><meta charset='utf-8'><title>FormFactor Certificate</title></head><body style='background:#050807;color:#dfffe0;font-family:monospace;text-align:center;padding:70px'><div style='border:3px solid #39ff14;padding:60px'><h1 style='color:#39ff14'>FORMFACTOR</h1><h2>%s — %s</h2><p>This digital training completion certificate is awarded to</p><h1>%s</h1><p>for completing %d of 100 progressive PCB construction and troubleshooting lessons.</p><p>Each lesson is 1.25%% harder than the previous lesson, using a compounding difficulty curve.</p><p>FormFactor training completion certificate • not a third-party professional accreditation</p></div></body></html>" % [str(tier["grade"]), str(tier["title"]), name_text.xml_escape(), int(tier["lessons"])]
    file.store_string(html)
    file.close()
    return path

func debug_top_certificate_title() -> String:
    return str(ACADEMY_CURRICULUM.CERTIFICATE_TIERS[ACADEMY_CURRICULUM.CERTIFICATE_TIERS.size() - 1]["title"])
