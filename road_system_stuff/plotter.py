import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from matplotlib.patches import Patch

from building_colors import BUILDING_COLORS
from models import BuildingType


def plot_graph(drive_nodes, drive_edges, walk_nodes, walk_edges, buildings, path=None):
    fig, ax = plt.subplots(figsize=(14, 14))
    drive_map = {node.id: node for node in drive_nodes}
    walk_map = {node.id: node for node in walk_nodes}

    for building in buildings:
        ax.fill(
            [corner.x for corner in building.corners],
            [corner.y for corner in building.corners],
            color=BUILDING_COLORS.get(building.building_type, "#BBBBBB"),
            edgecolor="#444444", linewidth=0.3, alpha=0.7, zorder=0,
        )

    for edge in drive_edges:
        n1, n2 = drive_map[edge.nfrom], drive_map[edge.nto]
        ax.plot([n1.x, n2.x], [n1.y, n2.y], color="#808080", linewidth=0.7, zorder=1)

    for edge in walk_edges:
        n1, n2 = walk_map[edge.nfrom], walk_map[edge.nto]
        ax.plot([n1.x, n2.x], [n1.y, n2.y], color="#00AA66", linewidth=1.0, zorder=2)

    ax.scatter([node.x for node in drive_nodes], [node.y for node in drive_nodes], s=3, color="#FF4444", zorder=3)
    ax.scatter([node.x for node in walk_nodes], [node.y for node in walk_nodes], s=2, color="#0066FF", zorder=4)

    if path and len(path) > 1:
        path_nodes = [drive_map[node_id] for node_id in path]
        ax.plot([node.x for node in path_nodes], [node.y for node in path_nodes], color="#00AAFF", linewidth=2.5, zorder=5)
        ax.scatter(
            [path_nodes[0].x, path_nodes[-1].x],
            [path_nodes[0].y, path_nodes[-1].y],
            s=40, color=["#FFFF00", "#0000FF"], zorder=6,
        )

    ax.set_aspect("equal")
    ax.set_title("Road Network and Buildings")
    ax.set_xlabel("X (meters)")
    ax.set_ylabel("Y (meters)")
    ax.grid(False)

    legend = [
        Line2D([0], [0], color="#808080", lw=2, label="Driving roads"),
        Line2D([0], [0], color="#00AA66", lw=2, label="Walking paths"),
        Line2D([0], [0], color="#00AAFF", lw=2, label="Driving route"),
    ]

    present_types = {building.building_type for building in buildings}
    for building_type in BuildingType:
        if building_type in present_types:
            legend.append(Patch(facecolor=BUILDING_COLORS[building_type], label=building_type.value.capitalize()))

    ax.legend(handles=legend, loc="upper right", fontsize=8)
    plt.show()


def plot_lane_graph(lane_nodes, lane_edges):
    """Plot the generated traffic-lane centerline graph separately."""
    fig, ax = plt.subplots(figsize=(14, 14))
    lane_map = {node.id: node for node in lane_nodes}

    for edge in lane_edges:
        n1 = lane_map[edge.nfrom]
        n2 = lane_map[edge.nto]
        ax.plot(
            [n1.x, n2.x],
            [n1.y, n2.y],
            color="#FF9900",
            linewidth=1.2,
            zorder=2,
        )

    ax.scatter(
        [node.x for node in lane_nodes],
        [node.y for node in lane_nodes],
        s=2,
        color="#CC5500",
        zorder=3,
    )

    ax.set_aspect("equal")
    ax.set_title("Traffic Lane Graph")
    ax.set_xlabel("X (meters)")
    ax.set_ylabel("Y (meters)")
    ax.legend(
        handles=[Line2D([0], [0], color="#FF9900", lw=2, label="Traffic lanes")],
        loc="upper right",
        fontsize=8,
    )
    ax.grid(False)
    plt.show()
