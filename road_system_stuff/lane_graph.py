import math

from shapely.geometry import LineString

from models import Node, Edge
from progress import ProgressBar


TRAFFIC_LANE_WIDTH = 3.5


def _first_positive_int(value) -> int | None:
    if value is None:
        return None
    if isinstance(value, list):
        value = value[0] if value else None
    if value is None:
        return None
    try:
        count = int(str(value).split(";")[0].strip())
        return count if count > 0 else None
    except (TypeError, ValueError):
        return None


def lane_counts(data: dict) -> tuple[int, int]:
    """Return (forward_lane_count, backward_lane_count)."""
    oneway = str(data.get("oneway", "no")).lower()

    forward = _first_positive_int(data.get("lanes:forward"))
    backward = _first_positive_int(data.get("lanes:backward"))
    total = _first_positive_int(data.get("lanes"))

    if oneway in {"yes", "true", "1"}:
        forward = forward or total or 1
        return forward, 0

    if oneway == "-1":
        backward = backward or total or 1
        return 0, backward

    if forward is not None or backward is not None:
        forward = forward or (backward or 1)
        backward = backward or (forward or 1)
        return forward, backward

    if total is not None:
        forward = math.ceil(total / 2.0)
        backward = math.floor(total / 2.0)
        return max(1, forward), max(1, backward)

    highway = data.get("highway")
    highways = highway if isinstance(highway, list) else [highway]
    if any(value in {"motorway", "motorway_link", "trunk", "trunk_link"} for value in highways):
        return 2, 2
    return 1, 1


def coordinates_in_direction(graph, u, v, data):
    geometry = data.get("geometry")
    if geometry is None:
        return [(graph.nodes[u]["x"], graph.nodes[u]["y"]),
                (graph.nodes[v]["x"], graph.nodes[v]["y"])]

    coordinates = list(geometry.coords)
    if len(coordinates) < 2:
        return []

    ux, uy = graph.nodes[u]["x"], graph.nodes[u]["y"]
    vx, vy = graph.nodes[v]["x"], graph.nodes[v]["y"]
    start = coordinates[0]

    if (start[0] - vx) ** 2 + (start[1] - vy) ** 2 < (
        start[0] - ux
    ) ** 2 + (start[1] - uy) ** 2:
        coordinates.reverse()

    return coordinates


def turn_type(incoming_line: LineString, outgoing_line: LineString) -> str | None:
    incoming = list(incoming_line.coords)
    outgoing = list(outgoing_line.coords)

    if len(incoming) < 2 or len(outgoing) < 2:
        return None

    ax = incoming[-2]
    bx = incoming[-1]
    cx = outgoing[1]

    incoming_dx = bx[0] - ax[0]
    incoming_dy = bx[1] - ax[1]
    outgoing_dx = cx[0] - bx[0]
    outgoing_dy = cx[1] - bx[1]

    incoming_length = math.hypot(incoming_dx, incoming_dy)
    outgoing_length = math.hypot(outgoing_dx, outgoing_dy)
    if incoming_length == 0 or outgoing_length == 0:
        return None

    dot = (incoming_dx * outgoing_dx + incoming_dy * outgoing_dy) / (
        incoming_length * outgoing_length
    )
    dot = max(-1.0, min(1.0, dot))
    angle = math.degrees(math.acos(dot))
    cross = incoming_dx * outgoing_dy - incoming_dy * outgoing_dx

    if angle >= 150:
        return "u_turn"
    if angle >= 25:
        return "left" if cross > 0 else "right"
    return "through"


def parse_turn_lanes(data: dict, direction: str) -> list[str] | None:
    key = f"turn:lanes:{direction}" if direction in {"forward", "backward"} else None
    value = data.get(key) if key else None
    if value is None:
        value = data.get("turn:lanes")
    if value is None:
        return None
    if isinstance(value, list):
        value = value[0] if value else None
    if value is None:
        return None

    values = [part.strip().lower() for part in str(value).split("|")]
    if not values:
        return None
    return values


def allowed_turns(
    lane_index: int,
    lane_count: int,
    turn_restrictions: list[str] | None,
) -> set[str]:
    if turn_restrictions:
        if lane_index < len(turn_restrictions):
            restriction = turn_restrictions[lane_index]
            allowed = set()
            for name in restriction.split(";"):
                name = name.strip()
                if name in {"left", "slight_left", "sharp_left"}:
                    allowed.add("left")
                elif name in {"right", "slight_right", "sharp_right"}:
                    allowed.add("right")
                elif name in {"through", "straight"}:
                    allowed.add("through")
            return allowed or {"left", "through", "right"}

    allowed = {"through"}
    if lane_index == 0:
        allowed.add("left")
    if lane_index == lane_count - 1:
        allowed.add("right")
    return allowed


def create_direction_lane(
    nodes: list[Node],
    edges: list[Edge],
    edge_pairs: set[tuple[int, int]],
    position_nodes: dict[tuple[int, int], int],
    road_line: LineString,
    lane_count: int,
    lane_index: int,
    direction: str,
    origin_x: float,
    origin_y: float,
    data: dict,
):
    # Lane 0 is the leftmost lane and lane_count - 1 is the rightmost lane
    # in the direction of travel.
    if lane_count == 1:
        offset = TRAFFIC_LANE_WIDTH / 2.0
    else:
        offset = (lane_count - 1) / 2.0 - lane_index
        offset *= TRAFFIC_LANE_WIDTH

    lane_line = road_line
    if abs(offset) >= 1e-9:
        lane_line = road_line.parallel_offset(
            abs(offset),
            side="left" if offset > 0 else "right",
            join_style=2,
        )

    if lane_line.is_empty:
        return None
    if lane_line.geom_type != "LineString":
        lane_line = LineString(lane_line.geoms[0].coords)

    lane_nodes: list[int] = []
    position_spacing = 0.25
    for x, y in lane_line.coords:
        key = (
            round((x - origin_x) / position_spacing),
            round((y - origin_y) / position_spacing),
        )
        node_id = position_nodes.get(key)

        if node_id is None:
            node_id = len(nodes)
            nodes.append(Node(id=node_id, x=x - origin_x, y=y - origin_y))
            position_nodes[key] = node_id

        lane_nodes.append(node_id)

    for i in range(len(lane_nodes) - 1):
        edge_pair = (lane_nodes[i], lane_nodes[i + 1])
        if edge_pair in edge_pairs:
            continue
        edge_pairs.add(edge_pair)

        edges.append(Edge(
            id=len(edges),
            nfrom=lane_nodes[i],
            nto=lane_nodes[i + 1],
        ))

    return lane_nodes, lane_line, lane_index, lane_count, parse_turn_lanes(data, direction)


def reverse_line(line: LineString) -> LineString:
    return LineString(list(line.coords)[::-1])


def create_lane_graph(graph, origin_x: float, origin_y: float) -> tuple[list[Node], list[Edge]]:
    """Create one directed lane graph with legally valid turn connections."""
    nodes: list[Node] = []
    edges: list[Edge] = []

    incoming_segments: dict[int, list[tuple]] = {}
    outgoing_segments: dict[int, list[tuple]] = {}
    seen_road_pairs: set[tuple[int, int]] = set()
    position_nodes: dict[tuple[int, int], int] = {}
    edge_pairs: set[tuple[int, int]] = set()

    road_edges = list(graph.edges(keys=True, data=True))
    progress = ProgressBar("Processing traffic lanes", len(road_edges))
    progress.start()

    for road_index, (u, v, key, data) in enumerate(road_edges, start=1):
        progress.update(road_index)
        if str(data.get("oneway", "no")).lower() in {"yes", "true", "1", "-1"}:
            road_pair = (u, v, str(key))
        else:
            road_pair = (min(u, v), max(u, v))

        if road_pair in seen_road_pairs:
            continue
        seen_road_pairs.add(road_pair)

        coordinates = coordinates_in_direction(graph, u, v, data)
        if len(coordinates) < 2:
            continue

        forward_count, backward_count = lane_counts(data)
        oneway = str(data.get("oneway", "no")).lower()

        road_line = LineString(coordinates)

        if oneway in {"yes", "true", "1"}:
            directions = [(u, v, road_line, forward_count, "forward")]
        elif oneway == "-1":
            reversed_line = reverse_line(road_line)
            directions = [(v, u, reversed_line, backward_count, "backward")]
        else:
            directions = [
                (u, v, road_line, forward_count, "forward"),
                (v, u, reverse_line(road_line), backward_count, "backward"),
            ]

        for start_id, end_id, direction_line, count, direction_name in directions:
            if count <= 0:
                continue

            for lane_index in range(count):
                segment = create_direction_lane(
                    nodes,
                    edges,
                    edge_pairs,
                    position_nodes,
                    direction_line,
                    count,
                    lane_index,
                    direction_name,
                    origin_x,
                    origin_y,
                    data,
                )
                if segment is None:
                    continue

                lane_nodes, lane_line, lane_index, lane_count, restrictions = segment
                outgoing_segments.setdefault(start_id, []).append(
                    (lane_nodes, lane_line, lane_index, lane_count, restrictions)
                )
                incoming_segments.setdefault(end_id, []).append(
                    (lane_nodes, lane_line, lane_index, lane_count, restrictions)
                )

    progress.finish()

    turn_progress = ProgressBar("Connecting lane turns", len(incoming_segments))
    turn_progress.start()

    for turn_index, intersection_id in enumerate(set(incoming_segments) & set(outgoing_segments), start=1):
        turn_progress.update(turn_index)

        incoming = incoming_segments[intersection_id]
        outgoing = outgoing_segments[intersection_id]

        for incoming_nodes, incoming_line, lane_index, lane_count, restrictions in incoming:
            incoming_allowed = allowed_turns(lane_index, lane_count, restrictions)

            for outgoing_nodes, outgoing_line, _, _, _ in outgoing:
                movement = turn_type(incoming_line, outgoing_line)
                if movement is None or movement == "u_turn":
                    continue
                if movement not in incoming_allowed:
                    continue

                edge_pair = (incoming_nodes[-1], outgoing_nodes[0])
                if edge_pair in edge_pairs:
                    continue
                edge_pairs.add(edge_pair)

                edges.append(Edge(
                    id=len(edges),
                    nfrom=incoming_nodes[-1],
                    nto=outgoing_nodes[0],
                ))

    turn_progress.finish()

    return nodes, edges
