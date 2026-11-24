from __future__ import annotations

import networkx as nx
from shapely.geometry import LineString


SIDEWALK_OFFSET_METERS = 2.5


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


def add_sidewalk_segment(walk_graph, line: LineString, side: str, index: int) -> None:
    """Add a sidewalk as consecutive graph nodes with no skipped intermediate nodes."""
    coordinates = list(line.coords)
    if len(coordinates) < 2:
        return

    node_ids: list[int] = []
    for x, y in coordinates:
        node_id = walk_graph.number_of_nodes()
        while node_id in walk_graph:
            node_id += 1

        walk_graph.add_node(node_id, x=float(x), y=float(y))
        node_ids.append(node_id)

    for from_index in range(len(node_ids) - 1):
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
):
    """Build sidewalks only inside the city and beside driving-road edges."""
    clean_graph = nx.MultiDiGraph()
    clean_graph.graph.update(drive_graph.graph)

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

            sidewalk_line = resample_line(sidewalk_line)
            add_sidewalk_segment(clean_graph, sidewalk_line, side, drive_edge_index)

        drive_edge_index += 1

    # Add only explicitly mapped sidewalk/footway walking ways that are fully inside the city.
    for u, v, key, data in walk_graph.edges(keys=True, data=True):
        if data.get("footway") != "sidewalk" and data.get("sidewalk") is None:
            continue

        walking_line = edge_line(walk_graph, u, v, data)
        if walking_line is None or not city_boundary.covers(walking_line):
            continue

        coordinates = list(walking_line.coords)
        if len(coordinates) < 2:
            continue

        walking_line = resample_line(walking_line)
        add_sidewalk_segment(clean_graph, walking_line, "explicit", drive_edge_index)
        drive_edge_index += 1

    return clean_graph


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
