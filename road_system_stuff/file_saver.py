from models import Node, Edge, Building


def save_graph(nodes: list[Node], edges: list[Edge], path: str) -> None:
    with open(path, "wt", encoding="utf-8") as file:
        file.write(f"{len(nodes)} {len(edges)}\n")
        for node in nodes:
            file.write(f"{node.id} {node.x} {node.y}\n")
        for edge in edges:
            file.write(f"{edge.id} {edge.nfrom} {edge.nto}\n")


def save_buildings(buildings: list[Building], path: str) -> None:
    with open(path, "wt", encoding="utf-8") as file:
        file.write(f"{len(buildings)}\n")
        for building in buildings:
            file.write(f"{building.building_type} {len(building.corners)} ")
            for corner in building.corners:
                file.write(f"{corner.x} {corner.y} ")
            file.write("\n")
