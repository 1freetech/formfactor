using System;
using System.Collections.Generic;
using UnityEngine;

namespace FormFactor.UnityFrontEnd
{
    public enum ComponentKind
    {
        Resistor = 0,
        Capacitor = 1,
        Led = 2,
        Chip = 3,
        Power = 4,
        Connector = 5
    }

    public sealed class PlacedComponent : MonoBehaviour
    {
        public int Id { get; set; }
        public ComponentKind Kind { get; set; }
    }

    [DefaultExecutionOrder(-1000)]
    public sealed class FormFactorWorkbench3D : MonoBehaviour
    {
        private const float GridStep = 0.25f;
        private const float BoardWidth = 10.0f;
        private const float BoardDepth = 6.0f;
        private const int HistoryLimit = 64;

        private sealed class WireLink
        {
            public int A;
            public int B;
            public LineRenderer Line;
        }

        [Serializable]
        private struct PartState
        {
            public int Id;
            public ComponentKind Kind;
            public Vector3 Position;
        }

        [Serializable]
        private struct WireState
        {
            public int A;
            public int B;
        }

        private sealed class Snapshot
        {
            public readonly List<PartState> Parts = new List<PartState>();
            public readonly List<WireState> Wires = new List<WireState>();
            public int SelectedId = -1;
        }

        private readonly List<PlacedComponent> _parts = new List<PlacedComponent>();
        private readonly List<WireLink> _wires = new List<WireLink>();
        private readonly Stack<Snapshot> _undo = new Stack<Snapshot>();
        private readonly Stack<Snapshot> _redo = new Stack<Snapshot>();
        private readonly IEngineeringCoreBridge _engineeringCore = new UnavailableEngineeringCoreBridge();

        private Camera _camera;
        private GameObject _board;
        private GameObject _ghost;
        private ComponentKind _selectedKind = ComponentKind.Led;
        private PlacedComponent _selected;
        private PlacedComponent _wireStart;
        private bool _dragging;
        private Vector3 _dragOrigin;
        private Snapshot _dragBefore;
        private int _nextId = 1;
        private string _status = "READY — CLICK THE BOARD TO PLACE A PART.";

        private float _cameraYaw = 38.0f;
        private float _cameraPitch = 42.0f;
        private float _cameraDistance = 12.0f;
        private Vector3 _cameraTarget = Vector3.zero;

        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.AfterSceneLoad)]
        private static void EnsureWorkbenchExists()
        {
            if (FindFirstObjectByType<FormFactorWorkbench3D>() != null)
            {
                return;
            }

            var root = new GameObject("FormFactor Unity Workbench");
            root.AddComponent<FormFactorWorkbench3D>();
        }

        private void Awake()
        {
            BuildCamera();
            BuildLighting();
            BuildBoard();
            BuildGrid();
            CreateGhost();

            // Match the existing beginner loop: power is present, LED is the first part to place.
            CreatePlaced(ComponentKind.Power, new Vector3(-2.0f, 0.18f, 0.5f), _nextId++);
            _selectedKind = ComponentKind.Led;
            _status = "FORMFACTOR 3D WORKBENCH — PLACE AN LED, CONNECT IT, THEN TEST.";
        }

        private void Update()
        {
            UpdateCameraInput();
            UpdateGhost();
            HandleKeyboard();
            HandleMouse();
            RefreshWires();
        }

        private void BuildCamera()
        {
            _camera = Camera.main;
            if (_camera == null)
            {
                var cameraObject = new GameObject("Main Camera");
                cameraObject.tag = "MainCamera";
                _camera = cameraObject.AddComponent<Camera>();
            }

            _camera.clearFlags = CameraClearFlags.SolidColor;
            _camera.backgroundColor = new Color32(5, 8, 10, 255);
            _camera.fieldOfView = 48.0f;
            ApplyCameraTransform();
        }

        private static void BuildLighting()
        {
            if (FindFirstObjectByType<Light>() != null)
            {
                return;
            }

            var lightObject = new GameObject("Workbench Key Light");
            var light = lightObject.AddComponent<Light>();
            light.type = LightType.Directional;
            light.intensity = 1.15f;
            light.color = new Color(0.92f, 1.0f, 0.95f);
            lightObject.transform.rotation = Quaternion.Euler(48.0f, -28.0f, 0.0f);
        }

        private void BuildBoard()
        {
            _board = GameObject.CreatePrimitive(PrimitiveType.Cube);
            _board.name = "FormFactor PCB Work Surface";
            _board.transform.position = new Vector3(0.0f, -0.10f, 0.0f);
            _board.transform.localScale = new Vector3(BoardWidth, 0.20f, BoardDepth);
            SetObjectColor(_board, new Color32(18, 83, 58, 255));
        }

        private void BuildGrid()
        {
            var gridRoot = new GameObject("Workbench Grid");
            gridRoot.transform.SetParent(transform, false);

            for (float x = -BoardWidth * 0.5f; x <= BoardWidth * 0.5f + 0.001f; x += 1.0f)
            {
                CreateGuideLine(gridRoot.transform,
                    new Vector3(x, 0.012f, -BoardDepth * 0.5f),
                    new Vector3(x, 0.012f, BoardDepth * 0.5f));
            }

            for (float z = -BoardDepth * 0.5f; z <= BoardDepth * 0.5f + 0.001f; z += 1.0f)
            {
                CreateGuideLine(gridRoot.transform,
                    new Vector3(-BoardWidth * 0.5f, 0.012f, z),
                    new Vector3(BoardWidth * 0.5f, 0.012f, z));
            }
        }

        private void CreateGuideLine(Transform parent, Vector3 a, Vector3 b)
        {
            var go = new GameObject("Grid Line");
            go.transform.SetParent(parent, false);
            var line = go.AddComponent<LineRenderer>();
            line.useWorldSpace = true;
            line.positionCount = 2;
            line.SetPosition(0, a);
            line.SetPosition(1, b);
            line.startWidth = 0.012f;
            line.endWidth = 0.012f;
            line.material = NewMaterial(new Color32(34, 125, 88, 145));
        }

        private void CreateGhost()
        {
            _ghost = CreateVisual(_selectedKind, Vector3.zero, true);
            _ghost.name = "Placement Ghost";
        }

        private void UpdateGhost()
        {
            if (_ghost == null)
            {
                return;
            }

            if (!TryGetBoardPoint(out var point))
            {
                _ghost.SetActive(false);
                return;
            }

            _ghost.SetActive(true);
            var snapped = SnapAndClamp(point, _selectedKind);
            _ghost.transform.position = snapped;
            ApplyShape(_ghost, _selectedKind);
            var blocked = WouldOverlap(_selectedKind, snapped, null);
            SetObjectColor(_ghost, blocked
                ? new Color32(255, 70, 70, 210)
                : new Color32(57, 255, 20, 175));
        }

        private void HandleKeyboard()
        {
            var ctrl = Input.GetKey(KeyCode.LeftControl) || Input.GetKey(KeyCode.RightControl) ||
                       Input.GetKey(KeyCode.LeftCommand) || Input.GetKey(KeyCode.RightCommand);
            var shift = Input.GetKey(KeyCode.LeftShift) || Input.GetKey(KeyCode.RightShift);

            if (ctrl && Input.GetKeyDown(KeyCode.Z))
            {
                Undo();
                return;
            }

            if (ctrl && (Input.GetKeyDown(KeyCode.Y) || (shift && Input.GetKeyDown(KeyCode.Z))))
            {
                Redo();
                return;
            }

            if (ctrl && Input.GetKeyDown(KeyCode.D))
            {
                DuplicateSelected();
                return;
            }

            if (Input.GetKeyDown(KeyCode.E))
            {
                PickSamePart();
            }

            if (Input.GetKeyDown(KeyCode.X))
            {
                DisconnectSelected();
            }

            if (Input.GetKeyDown(KeyCode.Delete) || Input.GetKeyDown(KeyCode.Backspace))
            {
                DeleteSelected();
            }

            if (Input.GetKeyDown(KeyCode.V))
            {
                var result = _engineeringCore.Validate(_parts.Count, _wires.Count);
                _status = result.Message;
            }

            if (shift)
            {
                var dx = 0;
                var dz = 0;
                if (Input.GetKeyDown(KeyCode.LeftArrow)) dx = -1;
                if (Input.GetKeyDown(KeyCode.RightArrow)) dx = 1;
                if (Input.GetKeyDown(KeyCode.UpArrow)) dz = 1;
                if (Input.GetKeyDown(KeyCode.DownArrow)) dz = -1;
                if (dx != 0 || dz != 0) NudgeSelected(dx, dz);
            }

            if (Input.GetKeyDown(KeyCode.Alpha1)) SelectKind(ComponentKind.Resistor);
            if (Input.GetKeyDown(KeyCode.Alpha2)) SelectKind(ComponentKind.Capacitor);
            if (Input.GetKeyDown(KeyCode.Alpha3)) SelectKind(ComponentKind.Led);
            if (Input.GetKeyDown(KeyCode.Alpha4)) SelectKind(ComponentKind.Chip);
            if (Input.GetKeyDown(KeyCode.Alpha5)) SelectKind(ComponentKind.Power);
            if (Input.GetKeyDown(KeyCode.Alpha6)) SelectKind(ComponentKind.Connector);
        }

        private void HandleMouse()
        {
            if (Input.GetMouseButtonDown(0))
            {
                if (TryPickPart(out var picked))
                {
                    _selected = picked;
                    _dragging = true;
                    _dragOrigin = picked.transform.position;
                    _dragBefore = CaptureSnapshot();
                    _status = $"SELECTED {picked.Kind} #{picked.Id}. DRAG TO MOVE.";
                }
                else if (TryGetBoardPoint(out var point))
                {
                    PlaceSelected(point);
                }
            }

            if (_dragging && _selected != null && Input.GetMouseButton(0) && TryGetBoardPoint(out var dragPoint))
            {
                _selected.transform.position = SnapAndClamp(dragPoint, _selected.Kind);
            }

            if (_dragging && Input.GetMouseButtonUp(0))
            {
                FinishDrag();
            }

            if (Input.GetMouseButtonDown(1) && TryPickPart(out var wirePart))
            {
                BeginOrFinishWire(wirePart);
            }
        }

        private void UpdateCameraInput()
        {
            if (Input.GetMouseButton(2))
            {
                _cameraYaw += Input.GetAxis("Mouse X") * 3.5f;
                _cameraPitch -= Input.GetAxis("Mouse Y") * 3.0f;
                _cameraPitch = Mathf.Clamp(_cameraPitch, 18.0f, 78.0f);
            }

            _cameraDistance = Mathf.Clamp(_cameraDistance - Input.mouseScrollDelta.y * 0.75f, 6.0f, 20.0f);
            ApplyCameraTransform();
        }

        private void ApplyCameraTransform()
        {
            if (_camera == null) return;
            var rotation = Quaternion.Euler(_cameraPitch, _cameraYaw, 0.0f);
            _camera.transform.position = _cameraTarget + rotation * new Vector3(0.0f, 0.0f, -_cameraDistance);
            _camera.transform.LookAt(_cameraTarget);
        }

        private void PlaceSelected(Vector3 rawPoint)
        {
            var position = SnapAndClamp(rawPoint, _selectedKind);
            if (WouldOverlap(_selectedKind, position, null))
            {
                _status = "BLOCKED — PARTS CANNOT OVERLAP.";
                return;
            }

            RecordEdit();
            _selected = CreatePlaced(_selectedKind, position, _nextId++);
            _status = $"PLACED {_selected.Kind} #{_selected.Id}. CTRL+Z UNDOS IT.";
        }

        private PlacedComponent CreatePlaced(ComponentKind kind, Vector3 position, int id)
        {
            var go = CreateVisual(kind, position, false);
            go.name = $"{kind} #{id}";
            var part = go.AddComponent<PlacedComponent>();
            part.Id = id;
            part.Kind = kind;
            _parts.Add(part);
            return part;
        }

        private GameObject CreateVisual(ComponentKind kind, Vector3 position, bool ghost)
        {
            var primitive = kind switch
            {
                ComponentKind.Capacitor => PrimitiveType.Cylinder,
                ComponentKind.Led => PrimitiveType.Sphere,
                _ => PrimitiveType.Cube
            };

            var go = GameObject.CreatePrimitive(primitive);
            go.transform.position = position;
            ApplyShape(go, kind);
            SetObjectColor(go, ghost ? new Color32(57, 255, 20, 175) : ColorForKind(kind));

            if (ghost)
            {
                var collider = go.GetComponent<Collider>();
                if (collider != null) collider.enabled = false;
            }

            return go;
        }

        private static void ApplyShape(GameObject go, ComponentKind kind)
        {
            var footprint = Footprint(kind);
            var height = kind switch
            {
                ComponentKind.Led => 0.34f,
                ComponentKind.Capacitor => 0.48f,
                ComponentKind.Chip => 0.20f,
                ComponentKind.Power => 0.34f,
                _ => 0.24f
            };
            go.transform.localScale = new Vector3(footprint.x, height, footprint.y);
            var p = go.transform.position;
            go.transform.position = new Vector3(p.x, height * 0.5f + 0.02f, p.z);
        }

        private static Vector2 Footprint(ComponentKind kind)
        {
            return kind switch
            {
                ComponentKind.Resistor => new Vector2(0.85f, 0.32f),
                ComponentKind.Capacitor => new Vector2(0.48f, 0.48f),
                ComponentKind.Led => new Vector2(0.42f, 0.42f),
                ComponentKind.Chip => new Vector2(1.05f, 0.72f),
                ComponentKind.Power => new Vector2(1.15f, 0.82f),
                ComponentKind.Connector => new Vector2(0.92f, 0.46f),
                _ => new Vector2(0.60f, 0.60f)
            };
        }

        private static Color ColorForKind(ComponentKind kind)
        {
            return kind switch
            {
                ComponentKind.Resistor => new Color32(212, 182, 126, 255),
                ComponentKind.Capacitor => new Color32(60, 145, 230, 255),
                ComponentKind.Led => new Color32(57, 255, 20, 255),
                ComponentKind.Chip => new Color32(35, 39, 43, 255),
                ComponentKind.Power => new Color32(235, 187, 38, 255),
                ComponentKind.Connector => new Color32(185, 192, 198, 255),
                _ => Color.white
            };
        }

        private static Material NewMaterial(Color color)
        {
            var shader = Shader.Find("Universal Render Pipeline/Lit") ??
                         Shader.Find("Standard") ??
                         Shader.Find("Hidden/InternalErrorShader");
            var material = new Material(shader);
            material.color = color;
            return material;
        }

        private static void SetObjectColor(GameObject go, Color color)
        {
            var renderer = go.GetComponent<Renderer>();
            if (renderer != null)
            {
                renderer.material = NewMaterial(color);
            }
        }

        private Vector3 SnapAndClamp(Vector3 raw, ComponentKind kind)
        {
            var footprint = Footprint(kind);
            var x = Mathf.Round(raw.x / GridStep) * GridStep;
            var z = Mathf.Round(raw.z / GridStep) * GridStep;
            var maxX = BoardWidth * 0.5f - footprint.x * 0.5f;
            var maxZ = BoardDepth * 0.5f - footprint.y * 0.5f;
            return new Vector3(Mathf.Clamp(x, -maxX, maxX), 0.0f, Mathf.Clamp(z, -maxZ, maxZ));
        }

        private bool WouldOverlap(ComponentKind kind, Vector3 position, PlacedComponent ignore)
        {
            var a = Footprint(kind);
            foreach (var part in _parts)
            {
                if (part == null || part == ignore) continue;
                var b = Footprint(part.Kind);
                var p = part.transform.position;
                if (Mathf.Abs(position.x - p.x) < (a.x + b.x) * 0.5f &&
                    Mathf.Abs(position.z - p.z) < (a.y + b.y) * 0.5f)
                {
                    return true;
                }
            }
            return false;
        }

        private bool TryGetBoardPoint(out Vector3 point)
        {
            point = default;
            if (_camera == null) return false;
            var ray = _camera.ScreenPointToRay(Input.mousePosition);
            var plane = new Plane(Vector3.up, Vector3.zero);
            if (!plane.Raycast(ray, out var distance)) return false;
            point = ray.GetPoint(distance);
            return Mathf.Abs(point.x) <= BoardWidth * 0.65f && Mathf.Abs(point.z) <= BoardDepth * 0.65f;
        }

        private bool TryPickPart(out PlacedComponent part)
        {
            part = null;
            if (_camera == null) return false;
            var ray = _camera.ScreenPointToRay(Input.mousePosition);
            if (!Physics.Raycast(ray, out var hit, 100.0f)) return false;
            part = hit.collider.GetComponent<PlacedComponent>();
            return part != null;
        }

        private void FinishDrag()
        {
            if (!_dragging || _selected == null) return;
            _dragging = false;
            var final = _selected.transform.position;
            if (WouldOverlap(_selected.Kind, final, _selected))
            {
                _selected.transform.position = _dragOrigin;
                _status = "BLOCKED — MOVE RETURNED TO THE LAST GOOD POSITION.";
                return;
            }

            if ((final - _dragOrigin).sqrMagnitude > 0.0001f)
            {
                PushLimited(_undo, _dragBefore);
                _redo.Clear();
                _status = $"MOVED {_selected.Kind} #{_selected.Id}. CTRL+Z UNDOS IT.";
            }
        }

        private void NudgeSelected(int dx, int dz)
        {
            if (_selected == null)
            {
                _status = "SELECT A PART BEFORE NUDGING IT.";
                return;
            }

            var candidate = SnapAndClamp(
                _selected.transform.position + new Vector3(dx * GridStep, 0.0f, dz * GridStep),
                _selected.Kind);
            if (WouldOverlap(_selected.Kind, candidate, _selected))
            {
                _status = "NUDGE BLOCKED — ANOTHER PART IS IN THE WAY.";
                return;
            }

            RecordEdit();
            _selected.transform.position = candidate;
            _status = "NUDGED ONE GRID STEP.";
        }

        private void DuplicateSelected()
        {
            if (_selected == null)
            {
                _status = "SELECT A PART BEFORE DUPLICATING IT.";
                return;
            }

            var source = _selected;
            for (var radius = 1; radius <= 12; radius++)
            {
                for (var dz = -radius; dz <= radius; dz++)
                {
                    for (var dx = -radius; dx <= radius; dx++)
                    {
                        if (Mathf.Max(Mathf.Abs(dx), Mathf.Abs(dz)) != radius) continue;
                        var candidate = SnapAndClamp(source.transform.position +
                            new Vector3(dx * GridStep, 0.0f, dz * GridStep), source.Kind);
                        if (WouldOverlap(source.Kind, candidate, null)) continue;
                        RecordEdit();
                        _selected = CreatePlaced(source.Kind, candidate, _nextId++);
                        _status = "DUPLICATED PART. WIRES WERE NOT COPIED.";
                        return;
                    }
                }
            }
            _status = "DUPLICATE BLOCKED — NO OPEN GRID POSITION FOUND.";
        }

        private void PickSamePart()
        {
            if (_selected == null)
            {
                _status = "SELECT A PART BEFORE USING PICK SAME PART.";
                return;
            }
            SelectKind(_selected.Kind);
            _status = $"PICKED {_selectedKind}. CLICK AN OPEN SPOT TO PLACE ANOTHER.";
        }

        private void DeleteSelected()
        {
            if (_selected == null) return;
            RecordEdit();
            var id = _selected.Id;
            RemoveWiresFor(id);
            _parts.Remove(_selected);
            Destroy(_selected.gameObject);
            _selected = null;
            _status = "REMOVED PART. CTRL+Z RESTORES IT.";
        }

        private void DisconnectSelected()
        {
            if (_selected == null)
            {
                _status = "SELECT A PART BEFORE DISCONNECTING IT.";
                return;
            }
            var count = CountWiresFor(_selected.Id);
            if (count == 0)
            {
                _status = "THE SELECTED PART HAS NO VISUAL CONNECTIONS.";
                return;
            }
            RecordEdit();
            RemoveWiresFor(_selected.Id);
            _status = $"DISCONNECTED {count} VISUAL CONNECTION{(count == 1 ? string.Empty : "S")}.";
        }

        private void BeginOrFinishWire(PlacedComponent part)
        {
            if (_wireStart == null)
            {
                _wireStart = part;
                _status = $"WIRE START: {part.Kind} #{part.Id}. RIGHT-CLICK ANOTHER PART.";
                return;
            }

            if (_wireStart == part)
            {
                _wireStart = null;
                _status = "WIRE CANCELLED.";
                return;
            }

            if (WireExists(_wireStart.Id, part.Id))
            {
                _wireStart = null;
                _status = "THOSE PARTS ARE ALREADY VISUALLY CONNECTED.";
                return;
            }

            RecordEdit();
            CreateWire(_wireStart.Id, part.Id);
            _status = "VISUAL CONNECTION ADDED. PRESS V FOR ENGINEERING VALIDATION STATUS.";
            _wireStart = null;
        }

        private void CreateWire(int a, int b)
        {
            var go = new GameObject($"Wire {a}-{b}");
            var line = go.AddComponent<LineRenderer>();
            line.positionCount = 2;
            line.useWorldSpace = true;
            line.startWidth = 0.055f;
            line.endWidth = 0.055f;
            line.material = NewMaterial(new Color32(255, 190, 35, 255));
            _wires.Add(new WireLink { A = a, B = b, Line = line });
        }

        private void RefreshWires()
        {
            foreach (var wire in _wires)
            {
                if (wire.Line == null) continue;
                var a = FindPart(wire.A);
                var b = FindPart(wire.B);
                if (a == null || b == null) continue;
                wire.Line.SetPosition(0, a.transform.position + Vector3.up * 0.28f);
                wire.Line.SetPosition(1, b.transform.position + Vector3.up * 0.28f);
            }
        }

        private void RemoveWiresFor(int id)
        {
            for (var i = _wires.Count - 1; i >= 0; i--)
            {
                if (_wires[i].A != id && _wires[i].B != id) continue;
                if (_wires[i].Line != null) Destroy(_wires[i].Line.gameObject);
                _wires.RemoveAt(i);
            }
            if (_wireStart != null && _wireStart.Id == id) _wireStart = null;
        }

        private int CountWiresFor(int id)
        {
            var count = 0;
            foreach (var wire in _wires)
            {
                if (wire.A == id || wire.B == id) count++;
            }
            return count;
        }

        private bool WireExists(int a, int b)
        {
            foreach (var wire in _wires)
            {
                if ((wire.A == a && wire.B == b) || (wire.A == b && wire.B == a)) return true;
            }
            return false;
        }

        private PlacedComponent FindPart(int id)
        {
            foreach (var part in _parts)
            {
                if (part != null && part.Id == id) return part;
            }
            return null;
        }

        private void SelectKind(ComponentKind kind)
        {
            _selectedKind = kind;
            if (_ghost != null) ApplyShape(_ghost, kind);
        }

        private void RecordEdit()
        {
            PushLimited(_undo, CaptureSnapshot());
            _redo.Clear();
        }

        private Snapshot CaptureSnapshot()
        {
            var snapshot = new Snapshot();
            foreach (var part in _parts)
            {
                if (part == null) continue;
                snapshot.Parts.Add(new PartState
                {
                    Id = part.Id,
                    Kind = part.Kind,
                    Position = part.transform.position
                });
            }
            foreach (var wire in _wires)
            {
                snapshot.Wires.Add(new WireState { A = wire.A, B = wire.B });
            }
            snapshot.SelectedId = _selected != null ? _selected.Id : -1;
            return snapshot;
        }

        private static void PushLimited(Stack<Snapshot> stack, Snapshot snapshot)
        {
            if (snapshot == null) return;
            if (stack.Count < HistoryLimit)
            {
                stack.Push(snapshot);
                return;
            }

            var items = stack.ToArray();
            stack.Clear();
            for (var i = items.Length - 2; i >= 0; i--) stack.Push(items[i]);
            stack.Push(snapshot);
        }

        private void RestoreSnapshot(Snapshot snapshot)
        {
            foreach (var wire in _wires)
            {
                if (wire.Line != null) Destroy(wire.Line.gameObject);
            }
            _wires.Clear();

            foreach (var part in _parts)
            {
                if (part != null) Destroy(part.gameObject);
            }
            _parts.Clear();
            _selected = null;
            _wireStart = null;

            var maxId = 0;
            foreach (var state in snapshot.Parts)
            {
                var part = CreatePlaced(state.Kind, state.Position, state.Id);
                maxId = Mathf.Max(maxId, state.Id);
                if (state.Id == snapshot.SelectedId) _selected = part;
            }
            foreach (var wire in snapshot.Wires)
            {
                if (FindPart(wire.A) != null && FindPart(wire.B) != null) CreateWire(wire.A, wire.B);
            }
            _nextId = maxId + 1;
        }

        private void Undo()
        {
            if (_undo.Count == 0)
            {
                _status = "NOTHING TO UNDO.";
                return;
            }
            PushLimited(_redo, CaptureSnapshot());
            RestoreSnapshot(_undo.Pop());
            _status = "UNDO — BOARD STATE RESTORED. RETEST WHEN READY.";
        }

        private void Redo()
        {
            if (_redo.Count == 0)
            {
                _status = "NOTHING TO REDO.";
                return;
            }
            PushLimited(_undo, CaptureSnapshot());
            RestoreSnapshot(_redo.Pop());
            _status = "REDO — BOARD STATE RESTORED. RETEST WHEN READY.";
        }

        private void OnGUI()
        {
            GUILayout.BeginArea(new Rect(16, 16, 430, 260), GUI.skin.box);
            GUILayout.Label("FORMFACTOR — UNITY 3D WORKBENCH");
            GUILayout.Label("1 Resistor   2 Capacitor   3 LED   4 Chip   5 Power   6 Connector");
            GUILayout.Label("Left click: place/select/drag   Right click: connect");
            GUILayout.Label("Shift+Arrows: nudge   Ctrl+D: duplicate   E: pick same part");
            GUILayout.Label("X: disconnect   Ctrl+Z/Y: undo/redo   V: validate");
            GUILayout.Label("Middle mouse: orbit   Wheel: zoom");
            GUILayout.Space(8);
            GUILayout.Label($"Selected tool: {_selectedKind}");
            GUILayout.Label($"Parts: {_parts.Count}   Visual connections: {_wires.Count}");
            GUILayout.Space(8);
            GUILayout.Label(_status);
            GUILayout.Space(8);
            GUILayout.Label("ENGINEERING TRUTH: C++ CORE BRIDGE IS FAIL-CLOSED UNTIL NATIVE LINKING IS COMPLETE.");
            GUILayout.EndArea();
        }
    }
}
