from __future__ import annotations

import networkx as nx
from shapely.geometry import LineString, MultiLineString, Point
from shapely.ops import unary_union
from shapely.strtree import STRtree

import progress

SIDEWALK_OFFSET_METERS = 2.5
SIDEWALK_RESAMPLE_SPACING_METERS = 20.0
SIDEWALK_MERGE_DISTANCE_METERS = 8.0


def edge_line(graph, u, v, data) -> LineString | None:
    geometry = data.get("geometry")
    if geometry is not None:
        line = LineString(list(geometry.coords))
    else:
        line = LineString([
            (graph.nodes[u]["x"], graph.nodes[u]["y"]),
            (graph.nodes[v]["x"], graph.nodes[v]["y"]),
        ])

    if line.is_empty or line.length == 0:
        return None
    return line


def add_sidewalk_segment(
    walk_graph,
    line: LineString,
    side: str,
    index: int,
    position_nodes: dict[tuple[int, int], int],
    edge_pairs: set[tuple[int, int]],
) -> None:
    """Add a sidewalk as consecutive graph nodes with no skipped intermediate nodes."""
    coordinates = list(line.coords)
    if len(coordinates) < 2:
        return

    node_ids: list[int] = []
    merge_spacing = SIDEWALK_MERGE_DISTANCE_METERS

    for x, y in coordinates:
        key = (round(x / merge_spacing), round(y / merge_spacing))
        node_id = position_nodes.get(key)

        if node_id is None:
            node_id = walk_graph.number_of_nodes()
            while node_id in walk_graph:
                node_id += 1

            walk_graph.add_node(node_id, x=float(x), y=float(y))
            position_nodes[key] = node_id

        node_ids.append(node_id)

    for from_index in range(len(node_ids) - 1):
        first_id = node_ids[from_index]
        second_id = node_ids[from_index + 1]
        if first_id == second_id:
            continue

        pair = frozenset((first_id, second_id))
        if pair in edge_pairs:
            continue
        edge_pairs.add(pair)

        walk_graph.add_edge(
            node_ids[from_index],
            node_ids[from_index + 1],
            key=f"{side}_{index}_{from_index}",
            footway="sidewalk",
            geometry=LineString([
                coordinates[from_index],
                coordinates[from_index + 1],
            ]),
        )


def prepare_walk_graph(
    walk_graph,
    drive_graph,
    city_boundary,
    progress_callback=None,
):
    """Build sidewalks only inside the city and beside driving-road edges."""
    clean_graph = nx.MultiDiGraph()
    clean_graph.graph.update(drive_graph.graph)

    position_nodes: dict[tuple[int, int], int] = {}
    edge_pairs: set[frozenset[int]] = set()
    drive_lines = collect_drive_lines(drive_graph)
    road_tree = STRtree(drive_lines)
    road_buffer = unary_union([line.buffer(0.5) for line in drive_lines])

    drive_edge_index = 0

    for u, v, key, data in drive_graph.edges(keys=True, data=True):
        road_line = edge_line(drive_graph, u, v, data)
        if road_line is None:
            continue

        # Road segment must be entirely within the municipality boundary.
        if not city_boundary.covers(road_line):
            continue

        for side in ("left", "right"):
            try:
                sidewalk_line = road_line.parallel_offset(
                    SIDEWALK_OFFSET_METERS,
                    side=side,
                    join_style=2,
                )
            except TypeError:
                sidewalk_line = road_line.parallel_offset(
                    SIDEWALK_OFFSET_METERS,
                    side=side,
                )

            if sidewalk_line.is_empty:
                continue

            if sidewalk_line.geom_type == "MultiLineString":
                parts = sorted(sidewalk_line.geoms, key=lambda line: line.length, reverse=True)
                sidewalk_line = parts[0]
            if sidewalk_line.geom_type != "LineString":
                continue

            sidewalk_line = resample_line(sidewalk_line, SIDEWALK_RESAMPLE_SPACING_METERS)

            if sidewalk_segment_crosses_road(sidewalk_line, drive_lines, road_tree):
                continue

            add_sidewalk_segment(
                clean_graph,
                sidewalk_line,
                side,
                drive_edge_index,
                position_nodes,
                edge_pairs,
            )

        drive_edge_index += 1

    # Add only explicitly mapped sidewalk/footway walking ways that are fully inside the city.
    walk_edges = list(walk_graph.edges(keys=True, data=True))
    for walk_index, (u, v, key, data) in enumerate(walk_edges, start=1):
        if data.get("footway") != "sidewalk" and data.get("sidewalk") is None:
            continue

        walking_line = edge_line(walk_graph, u, v, data)
        if walking_line is None or not city_boundary.covers(walking_line):
            continue

        coordinates = list(walking_line.coords)
        if len(coordinates) < 2:
            continue

        walking_line = resample_line(walking_line, SIDEWALK_RESAMPLE_SPACING_METERS)

        if sidewalk_segment_crosses_road(walking_line, drive_lines, road_tree):
            continue

        add_sidewalk_segment(
            clean_graph,
            walking_line,
            "explicit",
            drive_edge_index,
            position_nodes,
            edge_pairs,
        )
        drive_edge_index += 1
        if progress_callback is not None:
            progress_callback(walk_index, f"processing edge {walk_index}")

    if progress_callback is not None:
        progress_callback(len(walk_edges), "walking edges processed")

    merge_disconnected_sidewalk_components(
        clean_graph,
        drive_lines=drive_lines,
        road_tree=road_tree,
    )
    remove_crossing_sidewalk_edges(clean_graph, road_buffer)

    isolated_nodes = [node for node, degree in dict(clean_graph.degree()).items() if degree == 0]
    clean_graph.remove_nodes_from(isolated_nodes)

    return clean_graph


def collect_drive_lines(drive_graph) -> list[LineString]:
    lines: list[LineString] = []
    for u, v, key, data in drive_graph.edges(keys=True, data=True):
        line = edge_line(drive_graph, u, v, data)
        if line is not None:
            lines.append(line)
    return lines


def sidewalk_segment_crosses_road(
    segment: LineString,
    drive_lines: list[LineString],
    road_tree: STRtree | None = None,
) -> bool:
    """Reject segments that cross or intersect a nearby driving-road line."""
    if road_tree is None:
        road_tree = STRtree(drive_lines)

    for possible_index in road_tree.query(segment):
        if segment.intersects(drive_lines[int(possible_index)]):
            return True
    return False


def merge_disconnected_sidewalk_components(
    graph,
    drive_lines: list[LineString] = None,
    merge_distance_m: float = 40.0,
    road_tree: STRtree | None = None,
) -> None:
    """Merge nearest nodes between disconnected sidewalk components."""

    while True:

        components = list(nx.weakly_connected_components(graph))
        if len(components) <= 1:
            break

        components.sort(key=len, reverse=True)
        largest = components[0]
        largest_nodes = list(largest)
        largest_points = [Point(graph.nodes[target]["x"], graph.nodes[target]["y"])
                          for target in largest_nodes]
        largest_tree = STRtree(largest_points)

        best_pair = None
        best_distance = float("inf")

        for component in components[1:]:
            for source in component:
                sx = graph.nodes[source]["x"]
                sy = graph.nodes[source]["y"]

                nearest_result = largest_tree.query_nearest(Point(sx, sy))
                if hasattr(nearest_result, "__len__"):
                    nearest_index = int(nearest_result[0])
                else:
                    nearest_index = int(nearest_result)

                target = largest_nodes[nearest_index]
                tx = graph.nodes[target]["x"]
                ty = graph.nodes[target]["y"]
                distance = ((sx - tx) ** 2 + (sy - ty) ** 2) ** 0.5
                connection = LineString([(sx, sy), (tx, ty)])

                if drive_lines is not None and sidewalk_segment_crosses_road(connection, drive_lines, road_tree):
                    continue

                if distance < best_distance:
                    best_distance = distance
                    best_pair = (source, target)

        if best_pair is None or best_distance > merge_distance_m:
            break

        source, target = best_pair
        if source == target:
            break

        nx.relabel_nodes(graph, {source: target}, copy=False)

    graph.remove_edges_from(list(nx.selfloop_edges(graph)))


def remove_crossing_sidewalk_edges(graph, road_buffer) -> None:
    """Remove edges that touch or come within the protected road buffer."""
    for u, v, key, data in list(graph.edges(keys=True, data=True)):
        segment = LineString([
            (graph.nodes[u]["x"], graph.nodes[u]["y"]),
            (graph.nodes[v]["x"], graph.nodes[v]["y"]),
        ])
        if segment.intersects(road_buffer):
            graph.remove_edge(u, v, key=key)


def resample_line(line: LineString, spacing_m: float = 5.0) -> LineString:
    """Create a dense LineString whose consecutive vertices are short gaps."""
    if line.length <= spacing_m:
        return line

    distances = [0.0]
    distance = spacing_m
    while distance < line.length:
        distances.append(distance)
        distance += spacing_m
    distances.append(line.length)

    coordinates = [line.interpolate(value).coords[0] for value in distances]
    if len(coordinates) < 2:
        return line
    return LineString(coordinates)
