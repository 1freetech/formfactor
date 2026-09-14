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
if failures:
    raise SystemExit("Retired project identity remains:\n" + "\n".join(failures))
print("FormFactor identity check: PASS")
