from pydantic import BaseModel
from typing import List


class ParkingStats(BaseModel):
    free_places: int
    occupied_places: int
    total_places: int


class RenderPlace(BaseModel):
    coords: List[List[int]]
    state: int


class AffineTransform(BaseModel):
    valid: bool
    matrix: list[list[float]] | None = None


class RenderResponse(BaseModel):
    places: list[RenderPlace]
    transform: AffineTransform


class ParkingMetrics(BaseModel):
    fps: float
    last_latency_ms: float
