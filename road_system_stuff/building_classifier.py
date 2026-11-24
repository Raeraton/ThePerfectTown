from models import BuildingType
from tag_helpers import clean_tag


def classify_building(data) -> BuildingType:
    building = clean_tag(data.get("building"))
    amenity = clean_tag(data.get("amenity"))
    office = clean_tag(data.get("office"))
    shop = clean_tag(data.get("shop"))
    leisure = clean_tag(data.get("leisure"))
    tourism = clean_tag(data.get("tourism"))
    healthcare = clean_tag(data.get("healthcare"))
    religion = clean_tag(data.get("religion"))
    craft = clean_tag(data.get("craft"))
    landuse = clean_tag(data.get("landuse"))
    military = clean_tag(data.get("military"))

    if amenity == "hospital" or healthcare == "hospital":
        return BuildingType.HOSPITAL
    if healthcare or amenity in {"clinic", "doctors", "dentist", "pharmacy", "nursing_home", "social_facility"}:
        return BuildingType.HEALTHCARE
    if amenity in {"university", "college"}:
        return BuildingType.UNIVERSITY
    if amenity == "school":
        return BuildingType.SCHOOL
    if amenity in {"kindergarten", "childcare"}:
        return BuildingType.KINDERGARTEN
    if amenity in {"library", "music_school"}:
        return BuildingType.EDUCATION

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

    if amenity in {"cinema", "theatre", "nightclub", "arts_centre", "community_centre", "casino", "events_venue"}:
        return BuildingType.CULTURAL
    if amenity in {"restaurant", "cafe", "bar", "pub", "fast_food", "food_court"}:
        return BuildingType.RESTAURANT
    if leisure in {"sports_centre", "stadium", "fitness_centre", "swimming_pool", "ice_rink", "horse_riding"}:
        return BuildingType.SPORTS
    if tourism in {"hotel", "motel", "hostel", "guest_house", "chalet", "apartment"}:
        return BuildingType.HOTEL
    if leisure:
        return BuildingType.ENTERTAINMENT

    if office:
        return BuildingType.OFFICE
    if shop:
        return BuildingType.RETAIL
    if craft:
        return BuildingType.INDUSTRIAL

    if building in {"transportation", "train_station"}:
        return BuildingType.TRANSPORT
    if building == "parking":
        return BuildingType.PARKING
    if amenity in {"bus_station", "ferry_terminal", "taxi"}:
        return BuildingType.TRANSPORT

    if amenity in {"townhall", "courthouse", "police", "fire_station", "post_office", "prison", "embassy"}:
        return BuildingType.GOVERNMENT
    if amenity in {"public_building", "social_facility"}:
        return BuildingType.PUBLIC

    if military or building == "bunker":
        return BuildingType.MILITARY

    if building in {"industrial", "factory", "manufacture"}:
        return BuildingType.INDUSTRIAL
    if building in {"warehouse", "hangar", "storage_tank"}:
        return BuildingType.WAREHOUSE
    if building in {"farm", "barn", "cowshed", "stable", "livestock", "greenhouse"}:
        return BuildingType.FARM
    if landuse in {"farmland", "farmyard", "orchard"}:
        return BuildingType.AGRICULTURAL

    if building in {"apartments", "residential"}:
        return BuildingType.APARTMENT
    if building in {"house", "detached", "semidetached_house", "bungalow", "terrace"}:
        return BuildingType.HOUSE
    if building == "dormitory":
        return BuildingType.DORMITORY
    if building in {"yes", "houseboat", "static_caravan", "cabin"}:
        return BuildingType.RESIDENTIAL

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
