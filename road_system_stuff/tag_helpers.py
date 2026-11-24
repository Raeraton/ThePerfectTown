import math
import re


def parse_float(value) -> float | None:
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


def parse_int(value) -> int | None:
    number = parse_float(value)
    return None if number is None else int(number)


def clean_tag(value):
    """Convert missing OSM values to None."""
    if value is None:
        return None

    if str(value).strip().lower() in {"nan", "<na>", "nat", "none"}:
        return None

    try:
        if isinstance(value, float) and math.isnan(value):
            return None
    except (TypeError, ValueError):
        pass

    return value
