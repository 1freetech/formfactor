import subprocess
from pathlib import Path

retired = (
    ("pcb" + "tech").encode(),
    ("PCB" + "Tech").encode(),
    ("PCB" + " Tech").encode(),
)
failures = []
tracked = subprocess.check_output(["git", "ls-files", "-z"]).split(b"\0")
for raw in tracked:
    if not raw:
        continue
    path_text = raw.decode()
    path_bytes = raw
    if any(token in path_bytes for token in retired):
        failures.append(f"path: {path_text}")
        continue
    path = Path(path_text)
    if not path.exists() or not path.is_file():
        continue
    data = path.read_bytes()
    if any(token in data for token in retired):
        failures.append(f"content: {path_text}")

version = Path("VERSION").read_text(encoding="utf-8").strip()
project_text = Path("godot/project.godot").read_text(encoding="utf-8")
expected_name = f'config/name="FormFactor {version}"'
if expected_name not in project_text:
    failures.append(
        "version mismatch: VERSION is "
        f"{version!r} but godot/project.godot does not contain {expected_name!r}"
    )

if failures:
    raise SystemExit("FormFactor identity check failed:\n" + "\n".join(failures))
print(f"FormFactor identity check: PASS ({version})")
