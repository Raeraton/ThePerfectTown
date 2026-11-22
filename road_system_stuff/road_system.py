
import os
import re
import math

import osmnx as ox
import matplotlib.pyplot as plt

from dataclasses import dataclass, field
from enum import Enum
from pyproj import Transformer
from matplotlib.lines import Line2D
from matplotlib.patches import Patch


# ==================================================
# GLOBAL COORDINATE SETTINGS
# ==================================================

DRIVE_CRS = None
ORIGIN_X = 0
ORIGIN_Y = 0
TRANSFORMER = None


# ==================================================
# BUILDING TYPES
# ==================================================

class BuildingType(Enum):

    # Residential
    RESIDENTIAL = "residential"
    APARTMENT = "apartment"
    HOUSE = "house"
    DORMITORY = "dormitory"

    # Commercial and industrial
    COMMERCIAL = "commercial"
    OFFICE = "office"
    RETAIL = "retail"
    INDUSTRIAL = "industrial"
    WAREHOUSE = "warehouse"
    FARM = "farm"
    AGRICULTURAL = "agricultural"

    # Education
    EDUCATION = "education"
    SCHOOL = "school"
    UNIVERSITY = "university"
    KINDERGARTEN = "kindergarten"

    # Healthcare
    HEALTHCARE = "healthcare"
    HOSPITAL = "hospital"
    CLINIC = "clinic"

    # Religious
    RELIGIOUS = "religious"
    CHURCH = "church"
    MOSQUE = "mosque"
    SYNAGOGUE = "synagogue"
    TEMPLE = "temple"

    # Entertainment and tourism
    ENTERTAINMENT = "entertainment"
    RESTAURANT = "restaurant"
    HOTEL = "hotel"
    SPORTS = "sports"
    CULTURAL = "cultural"

    # Public services
    PUBLIC = "public"
    GOVERNMENT = "government"
    CIVIC = "civic"

    # Transport
    TRANSPORT = "transport"
    PARKING = "parking"

    # Other
    MILITARY = "military"
    SERVICE = "service"
    STORAGE = "storage"
    OTHER = "other"


# ==================================================
# DATA CLASSES
# ==================================================

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
            raise RuntimeError(
                "Download the driving graph before creating coordinates."
            )

        projected_x, projected_y = TRANSFORMER.transform(lon, lat)

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

    building_type: BuildingType = BuildingType.RESIDENTIAL

    # Original OSM information
    osm_building: str | None = None
    name: str | None = None

    # Physical properties
    height: float | None = None
    levels: int | None = None

    # All available OSM tags
    osm_tags: dict = field(default_factory=dict)

    def __post_init__(self):

        if len(self.corners) < 3:
            raise ValueError(
                "A building must have at least 3 corners."
            )

        if not isinstance(self.building_type, BuildingType):
            raise TypeError(
                "building_type must be a BuildingType."
            )


# ==================================================
# COORDINATE HELPERS
# ==================================================

def parseFloat(value) -> float | None:
    """Extract a numeric value from an OSM tag."""

    if value is None:
        return None

    try:
        match = re.search(r"-?\d+(?:\.\d+)?", str(value))

        if match:
            return float(match.group())

    except (ValueError, TypeError):
        pass

    return None


def parseInt(value) -> int | None:
    """Extract an integer from an OSM tag."""

    number = parseFloat(value)

    if number is None:
        return None

    return int(number)


def cleanTag(value):
    """Convert missing OSM values to None."""

    if value is None:
        return None

    try:
        if isinstance(value, float) and math.isnan(value):
            return None
    except TypeError:
        pass

    return value


# ==================================================
# BUILDING CLASSIFICATION
# ==================================================

def classifyBuilding(data) -> BuildingType:
    """Determine a building's type using its OSM tags."""

    building = cleanTag(data.get("building"))
    amenity = cleanTag(data.get("amenity"))
    office = cleanTag(data.get("office"))
    shop = cleanTag(data.get("shop"))
    leisure = cleanTag(data.get("leisure"))
    tourism = cleanTag(data.get("tourism"))
    healthcare = cleanTag(data.get("healthcare"))
    religion = cleanTag(data.get("religion"))
    craft = cleanTag(data.get("craft"))
    landuse = cleanTag(data.get("landuse"))
    military = cleanTag(data.get("military"))

    # ----------------------------------------------
    # Healthcare
    # ----------------------------------------------

    if amenity == "hospital" or healthcare == "hospital":
        return BuildingType.HOSPITAL

    if healthcare or amenity in {
        "clinic", "doctors", "dentist", "pharmacy"
    }:
        return BuildingType.HEALTHCARE

    if amenity in {"nursing_home", "social_facility"}:
        return BuildingType.HEALTHCARE

    # ----------------------------------------------
    # Education
    # ----------------------------------------------

    if amenity in {"university", "college"}:
        return BuildingType.UNIVERSITY

    if amenity == "school":
        return BuildingType.SCHOOL

    if amenity in {"kindergarten", "childcare"}:
        return BuildingType.KINDERGARTEN

    if amenity in {"library", "music_school"}:
        return BuildingType.EDUCATION

    # ----------------------------------------------
    # Religious
    # ----------------------------------------------

    if building in {"church", "chapel", "cathedral"}:
        return BuildingType.CHURCH

    if building == "mosque":
        return BuildingType.MOSQUE

    if building == "synagogue":
        return BuildingType.SYNAGOGUE

    if building == "temple":
        return BuildingType.TEMPLE

    if religion:
        return BuildingType.RELIGIOUS

    # ----------------------------------------------
    # Entertainment and culture
    # ----------------------------------------------

    if amenity in {
        "cinema",
        "theatre",
        "nightclub",
        "arts_centre",
        "community_centre",
        "casino",
        "events_venue"
    }:
        return BuildingType.CULTURAL

    if amenity in {
        "restaurant",
        "cafe",
        "bar",
        "pub",
        "fast_food",
        "food_court"
    }:
        return BuildingType.RESTAURANT

    if leisure in {
        "sports_centre",
        "stadium",
        "fitness_centre",
        "swimming_pool",
        "ice_rink",
        "horse_riding"
    }:
        return BuildingType.SPORTS

    if tourism in {
        "hotel",
        "motel",
        "hostel",
        "guest_house",
        "chalet",
        "apartment"
    }:
        return BuildingType.HOTEL

    if leisure:
        return BuildingType.ENTERTAINMENT

    # ----------------------------------------------
    # Commercial
    # ----------------------------------------------

    if office:
        return BuildingType.OFFICE

    if shop:
        return BuildingType.RETAIL

    if craft:
        return BuildingType.INDUSTRIAL

    # ----------------------------------------------
    # Transport
    # ----------------------------------------------

    if building in {"transportation", "train_station"}:
        return BuildingType.TRANSPORT

    if building == "parking":
        return BuildingType.PARKING

    if amenity in {
        "bus_station",
        "ferry_terminal",
        "taxi"
    }:
        return BuildingType.TRANSPORT

    # ----------------------------------------------
    # Public services
    # ----------------------------------------------

    if amenity in {
        "townhall",
        "courthouse",
        "police",
        "fire_station",
        "post_office",
        "prison",
        "embassy"
    }:
        return BuildingType.GOVERNMENT

    if amenity in {
        "public_building",
        "social_facility"
    }:
        return BuildingType.PUBLIC

    # ----------------------------------------------
    # Military
    # ----------------------------------------------

    if military or building == "bunker":
        return BuildingType.MILITARY

    # ----------------------------------------------
    # Industrial and storage
    # ----------------------------------------------

    if building in {
        "industrial",
        "factory",
        "manufacture"
    }:
        return BuildingType.INDUSTRIAL

    if building in {
        "warehouse",
        "hangar",
        "storage_tank"
    }:
        return BuildingType.WAREHOUSE

    if building in {
        "farm",
        "barn",
        "cowshed",
        "stable",
        "livestock",
        "greenhouse"
    }:
        return BuildingType.FARM

    if landuse in {"farmland", "farmyard", "orchard"}:
        return BuildingType.AGRICULTURAL

    # ----------------------------------------------
    # Residential
    # ----------------------------------------------

    if building in {"apartments", "residential"}:
        return BuildingType.APARTMENT

    if building in {
        "house",
        "detached",
        "semidetached_house",
        "bungalow",
        "terrace"
    }:
        return BuildingType.HOUSE

    if building == "dormitory":
        return BuildingType.DORMITORY

    if building in {
        "yes",
        "houseboat",
        "static_caravan",
        "cabin"
    }:
        return BuildingType.RESIDENTIAL

    # ----------------------------------------------
    # Other known building types
    # ----------------------------------------------

    if building in {"commercial", "retail"}:
        return BuildingType.COMMERCIAL

    if building == "office":
        return BuildingType.OFFICE

    if building in {"civic", "public"}:
        return BuildingType.CIVIC

    if building == "service":
        return BuildingType.SERVICE

    if building in {"garage", "garages", "shed"}:
        return BuildingType.STORAGE

    return BuildingType.OTHER


# ==================================================
# GRAPH CONVERSION
# ==================================================

def convertGraph(G, min_x, min_y):
    """Convert an OSMnx graph into Node and Edge lists."""

    nodes: list[Node] = []
    edges: list[Edge] = []

    osm_to_node_id = {
        osm_id: new_id
        for new_id, osm_id in enumerate(G.nodes)
    }

    # Convert original nodes
    for osm_id, data in G.nodes(data=True):

        nodes.append(Node(
            id=osm_to_node_id[osm_id],
            x=data["x"] - min_x,
            y=data["y"] - min_y
        ))

    next_node_id = len(nodes)

    # Convert edges and add curvature nodes
    for u, v, key, data in G.edges(keys=True, data=True):

        geometry = data.get("geometry")

        if geometry is not None:

            coordinates = list(geometry.coords)

            ux, uy = G.nodes[u]["x"], G.nodes[u]["y"]
            vx, vy = G.nodes[v]["x"], G.nodes[v]["y"]

            start = coordinates[0]

            # Ensure geometry goes from u to v
            if (
                (start[0] - vx) ** 2 + (start[1] - vy) ** 2
                <
                (start[0] - ux) ** 2 + (start[1] - uy) ** 2
            ):
                coordinates.reverse()

        else:

            coordinates = [
                (G.nodes[u]["x"], G.nodes[u]["y"]),
                (G.nodes[v]["x"], G.nodes[v]["y"])
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


# ==================================================
# DOWNLOAD DRIVING GRAPH
# ==================================================

def downloadDriveGraph(city: str) -> tuple[list[Node], list[Edge]]:
    """Download and convert the driving network."""

    global DRIVE_CRS, ORIGIN_X, ORIGIN_Y, TRANSFORMER

    G = ox.graph_from_place(
        city,
        network_type="drive"
    )

    G = ox.project_graph(G)

    DRIVE_CRS = G.graph["crs"]

    ORIGIN_X = min(
        data["x"]
        for _, data in G.nodes(data=True)
    )

    ORIGIN_Y = min(
        data["y"]
        for _, data in G.nodes(data=True)
    )

    TRANSFORMER = Transformer.from_crs(
        "EPSG:4326",
        DRIVE_CRS,
        always_xy=True
    )

    return convertGraph(G, ORIGIN_X, ORIGIN_Y)


# ==================================================
# DOWNLOAD WALKING GRAPH
# ==================================================

def downloadWalk(city: str) -> tuple[list[Node], list[Edge]]:
    """Download and convert the walking network."""

    if DRIVE_CRS is None:
        raise RuntimeError(
            "Download the driving graph before downloading the walking graph."
        )

    G = ox.graph_from_place(
        city,
        network_type="walk"
    )

    G = ox.project_graph(
        G,
        to_crs=DRIVE_CRS
    )

    return convertGraph(G, ORIGIN_X, ORIGIN_Y)


# ==================================================
# DOWNLOAD BUILDINGS
# ==================================================

def downloadBuildings(city: str) -> list[Building]:
    """Download building footprints and convert them into Building objects."""

    if DRIVE_CRS is None:
        raise RuntimeError(
            "Download the driving graph before downloading buildings."
        )

    buildings_gdf = ox.features_from_place(
        city,
        tags={"building": True}
    )

    buildings_gdf = buildings_gdf.to_crs(DRIVE_CRS)

    buildings: list[Building] = []

    for _, data in buildings_gdf.iterrows():

        geometry = data.geometry

        if geometry is None or geometry.is_empty:
            continue

        # Handle MultiPolygons
        if geometry.geom_type == "MultiPolygon":

            geometry = max(
                geometry.geoms,
                key=lambda polygon: polygon.area
            )

        if geometry.geom_type != "Polygon":
            continue

        # Convert exterior corners
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

        # Extract OSM metadata
        building_type = classifyBuilding(data)

        name = cleanTag(data.get("name"))
        osm_building = cleanTag(data.get("building"))

        height = parseFloat(data.get("height"))
        levels = parseInt(data.get("building:levels"))

        # Preserve all available OSM tags
        osm_tags = {
            key: value
            for key, value in data.items()
            if key != "geometry" and cleanTag(value) is not None
        }

        buildings.append(Building(
            corners=corners,
            entrance=entrance,
            building_type=building_type,
            osm_building=osm_building,
            name=name,
            height=height,
            levels=levels,
            osm_tags=osm_tags
        ))

    return buildings


# ==================================================
# BUILDING COLORS
# ==================================================

BUILDING_COLORS = {

    # Residential
    BuildingType.RESIDENTIAL: "#D98282",
    BuildingType.APARTMENT: "#C96B6B",
    BuildingType.HOUSE: "#E69A9A",
    BuildingType.DORMITORY: "#B85C5C",

    # Commercial and industrial
    BuildingType.COMMERCIAL: "#729FCF",
    BuildingType.OFFICE: "#4D85C5",
    BuildingType.RETAIL: "#8AB4E0",
    BuildingType.INDUSTRIAL: "#526D82",
    BuildingType.WAREHOUSE: "#607D8B",
    BuildingType.FARM: "#A5A66A",
    BuildingType.AGRICULTURAL: "#C1B66A",

    # Education
    BuildingType.EDUCATION: "#9B8AC4",
    BuildingType.SCHOOL: "#A58BC7",
    BuildingType.UNIVERSITY: "#8064A2",
    BuildingType.KINDERGARTEN: "#C2A0D6",

    # Healthcare
    BuildingType.HEALTHCARE: "#E88C8C",
    BuildingType.HOSPITAL: "#D9534F",
    BuildingType.CLINIC: "#F0A0A0",

    # Religious
    BuildingType.RELIGIOUS: "#A88C65",
    BuildingType.CHURCH: "#B99A70",
    BuildingType.MOSQUE: "#8EAA91",
    BuildingType.SYNAGOGUE: "#8E9BC4",
    BuildingType.TEMPLE: "#C49B75",

    # Entertainment
    BuildingType.ENTERTAINMENT: "#F4D35E",
    BuildingType.RESTAURANT: "#F2B84B",
    BuildingType.HOTEL: "#E8A45B",
    BuildingType.SPORTS: "#B5C96A",
    BuildingType.CULTURAL: "#D5A5D5",

    # Public services
    BuildingType.PUBLIC: "#A0A0A0",
    BuildingType.GOVERNMENT: "#777777",
    BuildingType.CIVIC: "#929292",

    # Transport
    BuildingType.TRANSPORT: "#5F9EA0",
    BuildingType.PARKING: "#808080",

    # Other
    BuildingType.MILITARY: "#65734B",
    BuildingType.SERVICE: "#A0A0A0",
    BuildingType.STORAGE: "#8C8C8C",
    BuildingType.OTHER: "#BBBBBB"
}


# ==================================================
# PLOT GRAPH
# ==================================================

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

    drive_map = {
        node.id: node
        for node in drive_nodes
    }

    walk_map = {
        node.id: node
        for node in walk_nodes
    }

    # ----------------------------------------------
    # Draw buildings
    # ----------------------------------------------

    for building in buildings:

        color = BUILDING_COLORS.get(
            building.building_type,
            "#BBBBBB"
        )

        ax.fill(
            [corner.x for corner in building.corners],
            [corner.y for corner in building.corners],
            color=color,
            edgecolor="#444444",
            linewidth=0.3,
            alpha=0.7,
            zorder=0
        )

    # ----------------------------------------------
    # Draw driving edges
    # ----------------------------------------------

    for edge in drive_edges:

        n1 = drive_map[edge.nfrom]
        n2 = drive_map[edge.nto]

        ax.plot(
            [n1.x, n2.x],
            [n1.y, n2.y],
            color="#808080",
            linewidth=0.7,
            zorder=1
        )

    # ----------------------------------------------
    # Draw walking edges
    # ----------------------------------------------

    for edge in walk_edges:

        n1 = walk_map[edge.nfrom]
        n2 = walk_map[edge.nto]

        ax.plot(
            [n1.x, n2.x],
            [n1.y, n2.y],
            color="#00AA66",
            linewidth=1.0,
            zorder=2
        )

    # ----------------------------------------------
    # Draw driving nodes
    # ----------------------------------------------

    ax.scatter(
        [node.x for node in drive_nodes],
        [node.y for node in drive_nodes],
        s=3,
        color="#FF4444",
        zorder=3
    )

    # ----------------------------------------------
    # Draw walking nodes
    # ----------------------------------------------

    ax.scatter(
        [node.x for node in walk_nodes],
        [node.y for node in walk_nodes],
        s=2,
        color="#0066FF",
        zorder=4
    )

    # ----------------------------------------------
    # Draw optional path
    # ----------------------------------------------

    if path and len(path) > 1:

        path_nodes = [
            drive_map[node_id]
            for node_id in path
        ]

        ax.plot(
            [node.x for node in path_nodes],
            [node.y for node in path_nodes],
            color="#00AAFF",
            linewidth=2.5,
            zorder=5
        )

        ax.scatter(
            [path_nodes[0].x, path_nodes[-1].x],
            [path_nodes[0].y, path_nodes[-1].y],
            s=40,
            color=["#FFFF00", "#0000FF"],
            zorder=6
        )

    # ----------------------------------------------
    # Plot settings
    # ----------------------------------------------

    ax.set_aspect("equal")

    ax.set_title("Road Network and Buildings")
    ax.set_xlabel("X (meters)")
    ax.set_ylabel("Y (meters)")

    ax.grid(False)

    # ----------------------------------------------
    # Legend
    # ----------------------------------------------

    legend = [
        Line2D(
            [0], [0],
            color="#808080",
            lw=2,
            label="Driving roads"
        ),

        Line2D(
            [0], [0],
            color="#00AA66",
            lw=2,
            label="Walking paths"
        ),

        Line2D(
            [0], [0],
            color="#00AAFF",
            lw=2,
            label="Driving route"
        )
    ]

    # Only show building types actually present
    present_types = {
        building.building_type
        for building in buildings
    }

    for building_type in BuildingType:

        if building_type not in present_types:
            continue

        legend.append(
            Patch(
                facecolor=BUILDING_COLORS[building_type],
                label=building_type.value.capitalize()
            )
        )

    ax.legend(
        handles=legend,
        loc="upper right",
        fontsize=8
    )

    plt.show()


# ==================================================
# GRAPH SAVING
# ==================================================

def graphToTextFile(
    nodes: list[Node],
    edges: list[Edge],
    path: str
) -> None:

    with open(path, "wt") as file:

        file.write(
            f"{len(nodes)} {len(edges)}\n"
        )

        for node in nodes:

            file.write(
                f"{node.id} {node.x} {node.y}\n"
            )

        for edge in edges:

            file.write(
                f"{edge.id} {edge.nfrom} {edge.nto}\n"
            )


# ==================================================
# MAIN
# ==================================================

def main():

    city = "Barcs, Hungary"

    # Download driving graph
    drive_nodes, drive_edges = downloadDriveGraph(city)

    print(f"Driving nodes: {len(drive_nodes)}")
    print(f"Driving edges: {len(drive_edges)}")

    # Download walking graph
    walk_nodes, walk_edges = downloadWalk(city)

    print(f"Walking nodes: {len(walk_nodes)}")
    print(f"Walking edges: {len(walk_edges)}")

    # Download buildings
    buildings = downloadBuildings(city)

    print(f"Buildings: {len(buildings)}")

    # Print building statistics
    building_counts = {}

    for building in buildings:

        building_type = building.building_type

        building_counts[building_type] = (
            building_counts.get(building_type, 0) + 1
        )

    print("\nBuilding statistics:")

    for building_type, count in sorted(
        building_counts.items(),
        key=lambda item: item[0].value
    ):

        print(
            f"{building_type.value}: {count}"
        )

    # Save graphs
    graphToTextFile(
        drive_nodes,
        drive_edges,
        "drive.txt"
    )

    graphToTextFile(
        walk_nodes,
        walk_edges,
        "walk.txt"
    )

    # Visualize everything
    plotGraph(
        drive_nodes,
        drive_edges,
        walk_nodes,
        walk_edges,
        buildings
    )


if __name__ == "__main__":
    main()
