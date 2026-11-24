from dataclasses import dataclass, field
from enum import Enum


class BuildingType(Enum):
    RESIDENTIAL = "residential"
    APARTMENT = "apartment"
    HOUSE = "house"
    DORMITORY = "dormitory"
    COMMERCIAL = "commercial"
    OFFICE = "office"
    RETAIL = "retail"
    INDUSTRIAL = "industrial"
    WAREHOUSE = "warehouse"
    FARM = "farm"
    AGRICULTURAL = "agricultural"
    EDUCATION = "education"
    SCHOOL = "school"
    UNIVERSITY = "university"
    KINDERGARTEN = "kindergarten"
    HEALTHCARE = "healthcare"
    HOSPITAL = "hospital"
    CLINIC = "clinic"
    RELIGIOUS = "religious"
    CHURCH = "church"
    MOSQUE = "mosque"
    SYNAGOGUE = "synagogue"
    TEMPLE = "temple"
    ENTERTAINMENT = "entertainment"
    RESTAURANT = "restaurant"
    HOTEL = "hotel"
    SPORTS = "sports"
    CULTURAL = "cultural"
    PUBLIC = "public"
    GOVERNMENT = "government"
    CIVIC = "civic"
    TRANSPORT = "transport"
    PARKING = "parking"
    MILITARY = "military"
    SERVICE = "service"
    STORAGE = "storage"
    OTHER = "other"


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


@dataclass
class Building:
    corners: list[Coordinate]
    entrance: Coordinate
    building_type: BuildingType = BuildingType.OTHER
    osm_building: str | None = None
    name: str | None = None
    height: float | None = None
    levels: int | None = None
    osm_tags: dict = field(default_factory=dict)

    def __post_init__(self):
        if len(self.corners) < 3:
            raise ValueError("A building must have at least 3 corners.")
        if not isinstance(self.building_type, BuildingType):
            raise TypeError("building_type must be a BuildingType.")
