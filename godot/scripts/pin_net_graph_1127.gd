class_name PinNetGraph1127
extends RefCounted

# Small deterministic undirected graph used by the Godot editor for pin-level
# topology previews. It does not decide electrical validity; the C++ core does.

var _adjacency: Dictionary = {}

func clear() -> void:
    _adjacency.clear()

func add_pin(endpoint: String) -> bool:
    var key := endpoint.strip_edges()
    if key.is_empty():
        return false
    if not _adjacency.has(key):
        _adjacency[key] = []
    return true

func connect_pins(a: String, b: String) -> bool:
    if a.is_empty() or b.is_empty() or a == b:
        return false
    add_pin(a)
    add_pin(b)
    var a_peers: Array = _adjacency[a]
    var b_peers: Array = _adjacency[b]
    if not a_peers.has(b):
        a_peers.append(b)
        a_peers.sort()
        _adjacency[a] = a_peers
    if not b_peers.has(a):
        b_peers.append(a)
        b_peers.sort()
        _adjacency[b] = b_peers
    return true

func disconnect_pins(a: String, b: String) -> void:
    if _adjacency.has(a):
        var a_peers: Array = _adjacency[a]
        a_peers.erase(b)
        _adjacency[a] = a_peers
    if _adjacency.has(b):
        var b_peers: Array = _adjacency[b]
        b_peers.erase(a)
        _adjacency[b] = b_peers

func has_pin(endpoint: String) -> bool:
    return _adjacency.has(endpoint)

func neighbors(endpoint: String) -> Array:
    if not _adjacency.has(endpoint):
        return []
    return (_adjacency[endpoint] as Array).duplicate()

func shortest_path(start: String, finish: String) -> PackedStringArray:
    if not has_pin(start) or not has_pin(finish):
        return PackedStringArray()
    var queue: Array[String] = [start]
    var visited: Dictionary = {start: true}
    var previous: Dictionary = {}

    while not queue.is_empty():
        var current := queue.pop_front()
        if current == finish:
            break
        var peers: Array = _adjacency[current]
        for peer_variant in peers:
            var peer := str(peer_variant)
            if visited.has(peer):
                continue
            visited[peer] = true
            previous[peer] = current
            queue.append(peer)

    if not visited.has(finish):
        return PackedStringArray()

    var path: Array[String] = [finish]
    var cursor := finish
    while cursor != start:
        cursor = str(previous[cursor])
        path.push_front(cursor)
    return PackedStringArray(path)

func connected_components() -> Array:
    var result: Array = []
    var keys: Array = _adjacency.keys()
    keys.sort()
    var visited: Dictionary = {}

    for start_variant in keys:
        var start := str(start_variant)
        if visited.has(start):
            continue
        var queue: Array[String] = [start]
        var component: Array[String] = []
        visited[start] = true
        while not queue.is_empty():
            var current := queue.pop_front()
            component.append(current)
            var peers: Array = _adjacency[current]
            for peer_variant in peers:
                var peer := str(peer_variant)
                if visited.has(peer):
                    continue
                visited[peer] = true
                queue.append(peer)
        component.sort()
        result.append(component)

    result.sort_custom(func(a: Array, b: Array) -> bool:
        if a.is_empty():
            return not b.is_empty()
        if b.is_empty():
            return false
        return str(a[0]) < str(b[0])
    )
    return result

func named_nets() -> Array[Dictionary]:
    var result: Array[Dictionary] = []
    var components := connected_components()
    for index in range(components.size()):
        result.append({
            "name": "NET%03d" % (index + 1),
            "pins": (components[index] as Array).duplicate()
        })
    return result

func edge_count() -> int:
    var total := 0
    for peers_variant in _adjacency.values():
        total += (peers_variant as Array).size()
    return total / 2

func pin_count() -> int:
    return _adjacency.size()
