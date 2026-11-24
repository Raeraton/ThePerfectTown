from models import Node, Edge


def convert_graph(graph, origin_x: float, origin_y: float) -> tuple[list[Node], list[Edge]]:
    """Convert an OSMnx graph into application Node and Edge lists."""
    nodes: list[Node] = []
    edges: list[Edge] = []

    osm_to_node_id = {
        osm_id: new_id
        for new_id, osm_id in enumerate(graph.nodes)
    }

    for osm_id, data in graph.nodes(data=True):
        nodes.append(Node(
            id=osm_to_node_id[osm_id],
            x=data["x"] - origin_x,
            y=data["y"] - origin_y,
        ))

    next_node_id = len(nodes)

    for u, v, key, data in graph.edges(keys=True, data=True):
        geometry = data.get("geometry")

        if geometry is not None:
            coordinates = list(geometry.coords)
            ux, uy = graph.nodes[u]["x"], graph.nodes[u]["y"]
            vx, vy = graph.nodes[v]["x"], graph.nodes[v]["y"]
            start = coordinates[0]
            if (
                (start[0] - vx) ** 2 + (start[1] - vy) ** 2
                < (start[0] - ux) ** 2 + (start[1] - uy) ** 2
            ):
                coordinates.reverse()
        else:
            coordinates = [
                (graph.nodes[u]["x"], graph.nodes[u]["y"]),
                (graph.nodes[v]["x"], graph.nodes[v]["y"]),
            ]

        road_nodes = [osm_to_node_id[u]]

        for x, y, *_ in coordinates[1:-1]:
            node_id = next_node_id
            next_node_id += 1
            nodes.append(Node(id=node_id, x=x - origin_x, y=y - origin_y))
            road_nodes.append(node_id)

        road_nodes.append(osm_to_node_id[v])

        for i in range(len(road_nodes) - 1):
            edges.append(Edge(
                id=len(edges),
                nfrom=road_nodes[i],
                nto=road_nodes[i + 1],
            ))

    return nodes, edges
