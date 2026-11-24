from models import Node, Edge, Building
from progress import ProgressBar


def save_graph(nodes: list[Node], edges: list[Edge], path: str) -> None:
    progress = ProgressBar(f"Saving {path}", len(nodes) + len(edges))
    progress.start()

    with open(path, "wt", encoding="utf-8") as file:
        file.write(f"{len(nodes)} {len(edges)}\n")
        saved = 0
        for node in nodes:
            file.write(f"{node.id} {node.x} {node.y}\n")
            saved += 1
            progress.update(saved)
        for edge in edges:
            file.write(f"{edge.id} {edge.nfrom} {edge.nto}\n")
            saved += 1
            progress.update(saved)

    progress.finish()


def save_buildings(buildings: list[Building], path: str) -> None:
    progress = ProgressBar(f"Saving {path}", len(buildings))
    progress.start()

    with open(path, "wt", encoding="utf-8") as file:
        file.write(f"{len(buildings)}\n")
        for building_index, building in enumerate(buildings, start=1):
            file.write(f"{building.building_type} {len(building.corners)} ")
            for corner in building.corners:
                file.write(f"{corner.x} {corner.y} ")
            file.write("\n")
            progress.update(building_index)

    progress.finish()
