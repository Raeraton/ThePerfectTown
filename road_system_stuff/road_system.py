
import osmnx as ox
from dataclasses import dataclass, field
import matplotlib.pyplot as plt

from pyproj import Transformer

DRIVE_CRS = None
ORIGIN_X = 0
ORIGIN_Y = 0
TRANSFORMER = None

@dataclass
class Node:
    id: int
    x: float
    y: float


@dataclass
class Edge:
    id: int
    nfrom: int
    nto: int


@dataclass
class Coordinate:
    x: float
    y: float

    def __init__(self, lat: float, lon: float):
        if TRANSFORMER is None:
            raise RuntimeError("Download the driving graph before creating coordinates.")

        # Convert WGS84 latitude/longitude to the graph's projected CRS
        projected_x, projected_y = TRANSFORMER.transform(lon, lat)

        # Use the same origin as the road network
        self.x = projected_x - ORIGIN_X
        self.y = projected_y - ORIGIN_Y

    @classmethod
    def from_projected(cls, x: float, y: float):
        coordinate = cls.__new__(cls)
        coordinate.x = x - ORIGIN_X
        coordinate.y = y - ORIGIN_Y
        return coordinate


@dataclass
class Building:
    corners: list[Coordinate]
    entrance: Coordinate
    building_type: str = "entertainment"

    VALID_TYPES = {"workplace", "residence", "entertainment"}

    def __post_init__(self):
        if self.building_type not in self.VALID_TYPES:
            raise ValueError(
                f"Invalid building type: {self.building_type}. "
                f"Expected one of: {', '.join(self.VALID_TYPES)}"
            )

        if len(self.corners) < 3:
            raise ValueError("A building must have at least 3 corners.")


def convertGraph(G, min_x, min_y):
    """Convert an OSMnx graph into Node and Edge lists."""

    nodes: list[Node] = []
    edges: list[Edge] = []

    # Map OSM IDs to sequential IDs
    osm_to_node_id = {
        osm_id: new_id
        for new_id, osm_id in enumerate(G.nodes)
    }

    # Convert original nodes
    for osm_id, data in G.nodes(data=True):
        nodes.append(Node(
            id=osm_to_node_id[osm_id],
            x=data['x'] - min_x,
            y=data['y'] - min_y
        ))

    next_node_id = len(nodes)

    # Convert edges and add curvature nodes
    for u, v, key, data in G.edges(keys=True, data=True):

        geometry = data.get('geometry')

        if geometry is not None:
            coordinates = list(geometry.coords)

            # Ensure geometry goes from u to v
            ux, uy = G.nodes[u]['x'], G.nodes[u]['y']
            vx, vy = G.nodes[v]['x'], G.nodes[v]['y']

            start = coordinates[0]
            end = coordinates[-1]

            if ((start[0] - vx)**2 + (start[1] - vy)**2 <
                (start[0] - ux)**2 + (start[1] - uy)**2):
                coordinates.reverse()

        else:
            coordinates = [
                (G.nodes[u]['x'], G.nodes[u]['y']),
                (G.nodes[v]['x'], G.nodes[v]['y'])
            ]

        road_nodes = [osm_to_node_id[u]]

        # Add intermediate curvature nodes
        for x, y, *rest in coordinates[1:-1]:

            node_id = next_node_id
            next_node_id += 1

            nodes.append(Node(
                id=node_id,
                x=x - min_x,
                y=y - min_y
            ))

            road_nodes.append(node_id)

        road_nodes.append(osm_to_node_id[v])

        # Connect consecutive nodes
        for i in range(len(road_nodes) - 1):
            edges.append(Edge(
                id=len(edges),
                nfrom=road_nodes[i],
                nto=road_nodes[i + 1]
            ))

    return nodes, edges

def downloadDriveGraph(city: str) -> tuple[list[Node], list[Edge]]:
    """Download and convert the driving network."""

    global DRIVE_CRS, ORIGIN_X, ORIGIN_Y, TRANSFORMER

    G = ox.graph_from_place(city, network_type='drive')
    G = ox.project_graph(G)

    DRIVE_CRS = G.graph['crs']

    ORIGIN_X = min(data['x'] for _, data in G.nodes(data=True))
    ORIGIN_Y = min(data['y'] for _, data in G.nodes(data=True))

    TRANSFORMER = Transformer.from_crs(
        "EPSG:4326",
        DRIVE_CRS,
        always_xy=True
    )

    return convertGraph(G, ORIGIN_X, ORIGIN_Y)


def downloadWalk(city: str) -> tuple[list[Node], list[Edge]]:
    """Download and convert the walking network."""

    # Download walking graph
    G = ox.graph_from_place(city, network_type='walk')

    # Project to the same CRS as the driving graph
    G = ox.project_graph(G, to_crs=DRIVE_CRS)

    return convertGraph(G, ORIGIN_X, ORIGIN_Y)


def downloadBuildings(city: str) -> list[Building]:
    """Download building footprints and convert them into Building objects."""

    if DRIVE_CRS is None:
        raise RuntimeError("Download the driving graph before downloading buildings.")

    # Download all buildings
    buildings_gdf = ox.features_from_place(
        city,
        tags={"building": True}
    )

    # Convert to the same CRS as the road network
    buildings_gdf = buildings_gdf.to_crs(DRIVE_CRS)

    buildings: list[Building] = []

    for _, data in buildings_gdf.iterrows():

        geometry = data.geometry

        # Ignore unsupported geometries
        if geometry is None or geometry.is_empty:
            continue

        # Handle multipolygons by selecting the largest polygon
        if geometry.geom_type == "MultiPolygon":
            geometry = max(geometry.geoms, key=lambda p: p.area)

        if geometry.geom_type != "Polygon":
            continue

        # Convert polygon corners
        corners = [
            Coordinate.from_projected(x, y)
            for x, y in list(geometry.exterior.coords)[:-1]
        ]

        if len(corners) < 3:
            continue

        # Approximate entrance using a point inside the building
        point = geometry.representative_point()

        entrance = Coordinate.from_projected(
            point.x,
            point.y
        )

        # Determine building type from OSM tags
        building_tag = data.get("building")
        amenity = data.get("amenity")
        office = data.get("office")
        leisure = data.get("leisure")

        building_type = "residence"

        entertainment_tags = {
            "restaurant", "cafe", "bar", "pub",
            "cinema", "theatre", "nightclub",
            "arts_centre", "community_centre"
        }

        workplace_tags = {
            "commercial", "industrial", "retail",
            "warehouse", "factory", "office"
        }

        if isinstance(amenity, str) and amenity in entertainment_tags:
            building_type = "entertainment"

        elif isinstance(leisure, str) and leisure in {
            "sports_centre", "stadium", "fitness_centre"
        }:
            building_type = "entertainment"

        elif isinstance(office, str):
            building_type = "workplace"

        elif isinstance(building_tag, str) and building_tag in workplace_tags:
            building_type = "workplace"

        buildings.append(Building(
            corners=corners,
            entrance=entrance,
            building_type=building_type
        ))

    return buildings

#....................................................
# plot
#.....................................................

def plotGraph(
    drive_nodes: list[Node],
    drive_edges: list[Edge],
    walk_nodes: list[Node],
    walk_edges: list[Edge],
    buildings: list[Building],
    path: list[int] | None = None
) -> None:
    """Plot driving and walking networks together with buildings."""

    fig, ax = plt.subplots(figsize=(14, 14))

    drive_map = {node.id: node for node in drive_nodes}
    walk_map = {node.id: node for node in walk_nodes}

    building_colors = {
        "residence": "#D98282",
        "workplace": "#729FCF",
        "entertainment": "#F4D35E"
    }

    # Draw buildings
    for building in buildings:
        color = building_colors[building.building_type]

        ax.fill(
            [corner.x for corner in building.corners],
            [corner.y for corner in building.corners],
            color=color,
            edgecolor="#444444",
            linewidth=0.3,
            alpha=0.7,
            zorder=0
        )

    # Draw driving edges
    for edge in drive_edges:
        n1 = drive_map[edge.nfrom]
        n2 = drive_map[edge.nto]

        ax.plot(
            [n1.x, n2.x],
            [n1.y, n2.y],
            color='#808080',
            linewidth=0.7,
            zorder=1
        )

    # Draw walking edges
    for edge in walk_edges:
        n1 = walk_map[edge.nfrom]
        n2 = walk_map[edge.nto]

        ax.plot(
            [n1.x, n2.x],
            [n1.y, n2.y],
            color='#00AA66',
            linewidth=1.0,
            zorder=2
        )

    # Draw driving nodes
    ax.scatter(
        [node.x for node in drive_nodes],
        [node.y for node in drive_nodes],
        s=3,
        color='#FF4444',
        zorder=3
    )

    # Draw walking nodes
    ax.scatter(
        [node.x for node in walk_nodes],
        [node.y for node in walk_nodes],
        s=2,
        color='#0066FF',
        zorder=4
    )

    # Draw optional path (using driving node IDs)
    if path and len(path) > 1:
        path_nodes = [drive_map[node_id] for node_id in path]

        ax.plot(
            [node.x for node in path_nodes],
            [node.y for node in path_nodes],
            color='#00AAFF',
            linewidth=2.5,
            zorder=5
        )

        ax.scatter(
            [path_nodes[0].x, path_nodes[-1].x],
            [path_nodes[0].y, path_nodes[-1].y],
            s=40,
            color=['#FFFF00', '#0000FF'],
            zorder=6
        )

    ax.set_aspect('equal')
    ax.set_title('Road Network and Buildings')
    ax.set_xlabel('X (meters)')
    ax.set_ylabel('Y (meters)')
    ax.grid(False)

    # Legend
    from matplotlib.lines import Line2D
    from matplotlib.patches import Patch

    legend = [
        Line2D([0], [0], color='#808080', lw=2, label='Driving roads'),
        Line2D([0], [0], color='#00AA66', lw=2, label='Walking paths'),
        Line2D([0], [0], color='#00AAFF', lw=2, label='Driving route'),
        Patch(facecolor='#D98282', label='Residence'),
        Patch(facecolor='#729FCF', label='Workplace'),
        Patch(facecolor='#F4D35E', label='Entertainment'),
    ]

    ax.legend(handles=legend)

    plt.show()


# --------------------------------------------------
# Saving (unchanged)
# --------------------------------------------------

def graphToTextFile(nodes: list[Node], edges: list[Edge], path: str) -> None:
    with open(path, "wt") as file:
        file.write(f"{len(nodes)} {len(edges)}\n")

        for node in nodes:
            file.write(f"{node.id} {node.x} {node.y}\n")

        for edge in edges:
            file.write(f"{edge.id} {edge.nfrom} {edge.nto}\n")


# --------------------------------------------------
# Main
# --------------------------------------------------


city = 'Barcs, Hungary'

drive_nodes, drive_edges = downloadDriveGraph(city)
print(f"Driving nodes: {len(drive_nodes)}")
print(f"Driving edges: {len(drive_edges)}")

walk_nodes, walk_edges = downloadWalk(city)
print(f"Walking nodes: {len(walk_nodes)}")
print(f"Walking edges: {len(walk_edges)}")


buildings = downloadBuildings(city)
print(f"Buildings: {len(buildings)}")


# Save driving graph (same format as before)
graphToTextFile(drive_nodes, drive_edges, "drive.txt")
graphToTextFile(walk_nodes, walk_edges, "walk.txt")

# Visualize both
plotGraph(drive_nodes, drive_edges, walk_nodes, walk_edges, buildings)
