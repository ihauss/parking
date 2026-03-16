from pydantic import BaseModel
from typing import List, Optional
from enum import Enum

class CameraMetrics(BaseModel):
    fps: float
    latency_ms: float

class GlobalMetrics(BaseModel):
    camera_count: int
    healthy: int
    unhealthy: int
    avg_fps: float
    avg_latency_ms: float

class RenderPlaceModel(BaseModel):
    coords: list[tuple[int, int]]
    state: int


class AffineTransformModel(BaseModel):
    valid: bool
    matrix: list[float] | None


class SnapshotResponse(BaseModel):
    places: list[RenderPlaceModel]
    num_occupied: int
    num_places: int
    transform: AffineTransformModel


class ParkingStatsResponse(BaseModel):
    occupied: int
    free: int
    total: int


class CameraInfo(BaseModel):
    id: str
    healthy: bool


class HealthResponse(BaseModel):
    healthy: bool

class CameraStateResponse(BaseModel):
    state: str
    healthy: bool