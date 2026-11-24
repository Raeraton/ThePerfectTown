import osmnx as ox

from building_classifier import classify_building
from models import Building, Coordinate
from tag_helpers import parse_float, parse_int, clean_tag


def download_buildings(city: str, drive_crs, origin_x: float, origin_y: float, transformer):
    if drive_crs is None:
        raise RuntimeError("Download the driving graph before downloading buildings.")

    buildings_gdf = ox.features_from_place(city, tags={"building": True})
    buildings_gdf = buildings_gdf.to_crs(drive_crs)
    buildings: list[Building] = []

    for _, data in buildings_gdf.iterrows():
        geometry = data.geometry
        if geometry is None or geometry.is_empty:
            continue

        if geometry.geom_type == "MultiPolygon":
            geometry = max(geometry.geoms, key=lambda polygon: polygon.area)
        if geometry.geom_type != "Polygon":
            continue

        corners = [
            projected_coordinate(x, y, origin_x, origin_y)
            for x, y in list(geometry.exterior.coords)[:-1]
        ]
        if len(corners) < 3:
            continue

        point = geometry.representative_point()
        entrance = projected_coordinate(point.x, point.y, origin_x, origin_y)

        buildings.append(Building(
            corners=corners,
            entrance=entrance,
            building_type=classify_building(data),
            osm_building=clean_tag(data.get("building")),
            name=clean_tag(data.get("name")),
            height=parse_float(data.get("height")),
            levels=parse_int(data.get("building:levels")),
            osm_tags={
                key: value
                for key, value in data.items()
                if key != "geometry" and clean_tag(value) is not None
            },
        ))

    return buildings


def projected_coordinate(x: float, y: float, origin_x: float, origin_y: float) -> Coordinate:
    return Coordinate(x=x - origin_x, y=y - origin_y)
