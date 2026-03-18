from pydantic import BaseModel
from typing import List, Optional, Tuple
from enum import Enum


class CameraMetrics(BaseModel):
    """
    Performance metrics for a single camera.
    """
    fps: float
    latency_ms: float


class GlobalMetrics(BaseModel):
    """
    Aggregated metrics across all cameras.
    """
    camera_count: int
    healthy: int
    unhealthy: int
    avg_fps: float
    avg_latency_ms: float


class RenderPlaceModel(BaseModel):
    """
    Representation of a parking place for rendering.

    Attributes:
        coords: Polygon coordinates (list of (x, y) points)
        state: Occupancy state (e.g., 0 = free, 1 = occupied)
    """
    coords: List[Tuple[int, int]]
    state: int


class AffineTransformModel(BaseModel):
    """
    Affine transformation applied to the camera view.

    Attributes:
        valid: Whether the transform is valid
        matrix: Flattened transformation matrix (if available)
    """
    valid: bool
    matrix: Optional[List[float]]


class SnapshotResponse(BaseModel):
    """
    Snapshot of the parking state for a camera.

    Includes detected places, occupancy, and optional transform.
    """
    places: List[RenderPlaceModel]
    num_occupied: int
    num_places: int
    transform: AffineTransformModel


class ParkingStatsResponse(BaseModel):
    """
    Global parking statistics.
    """
    occupied: int
    free: int
    total: int


class CameraInfo(BaseModel):
    """
    Basic information about a camera.
    """
    id: str
    healthy: bool


class HealthResponse(BaseModel):
    """
    Health status response.
    """
    healthy: bool


class CameraStateResponse(BaseModel):
    """
    Detailed state of a camera.
    """
    state: str
    healthy: bool