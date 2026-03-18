"""
Metrics API.

Provides:
- Per-camera performance metrics
- Global aggregated metrics across all cameras
"""

from fastapi import APIRouter, Request, HTTPException
from typing import List
import time

from app.models import CameraMetrics, GlobalMetrics


router = APIRouter()


# =========================
# Per-camera metrics
# =========================

@router.get("/{camera_id}", response_model=CameraMetrics)
def get_camera_metrics(camera_id: str, request: Request):
    """
    Retrieve performance metrics for a single camera.
    """
    logger = request.app.state.logger
    start_time = time.perf_counter()

    parking_system = request.app.state.parking_system

    if parking_system is None:
        logger.error("Parking system not initialized")
        raise HTTPException(
            status_code=503,
            detail="Parking system not initialized"
        )

    try:
        stats = parking_system.get_stats(camera_id)

        elapsed_ms = (time.perf_counter() - start_time) * 1000

        logger.debug(
            "metrics/%s | fps=%.2f latency=%.2f | %.2f ms",
            camera_id,
            stats["fps"],
            stats["latency_ms"],
            elapsed_ms,
        )

        return CameraMetrics(
            fps=stats["fps"],
            latency_ms=stats["latency_ms"]
        )

    except RuntimeError as e:
        logger.warning(
            "metrics/%s not found: %s",
            camera_id,
            str(e)
        )
        raise HTTPException(
            status_code=404,
            detail=str(e)
        )

    except Exception:
        logger.exception(
            "metrics/%s unexpected error",
            camera_id
        )
        raise HTTPException(
            status_code=500,
            detail="Internal server error"
        )


# =========================
# Global aggregated metrics
# =========================

@router.get("/", response_model=GlobalMetrics)
def get_global_metrics(request: Request):
    """
    Retrieve aggregated metrics across all cameras.
    """
    logger = request.app.state.logger
    start_time = time.perf_counter()

    parking_system = request.app.state.parking_system

    if parking_system is None:
        logger.error("Parking system not initialized")
        raise HTTPException(
            status_code=503,
            detail="Parking system not initialized"
        )

    try:
        camera_ids: List[str] = parking_system.list_cameras()

        if not camera_ids:
            logger.debug("No cameras registered")

            return GlobalMetrics(
                camera_count=0,
                healthy=0,
                unhealthy=0,
                avg_fps=0.0,
                avg_latency_ms=0.0
            )

        fps_values = []
        latency_values = []
        healthy_count = 0
        effective_count = 0  # cameras successfully queried

        for camera_id in camera_ids:
            try:
                stats = parking_system.get_stats(camera_id)

                fps_values.append(stats["fps"])
                latency_values.append(stats["latency_ms"])
                effective_count += 1

                if parking_system.is_healthy(camera_id):
                    healthy_count += 1

            except RuntimeError:
                logger.warning(
                    "Camera %s unavailable during aggregation",
                    camera_id
                )
                continue

        camera_count = len(camera_ids)
        unhealthy_count = camera_count - healthy_count

        avg_fps = (
            sum(fps_values) / effective_count
            if effective_count > 0 else 0.0
        )

        avg_latency = (
            sum(latency_values) / effective_count
            if effective_count > 0 else 0.0
        )

        elapsed_ms = (time.perf_counter() - start_time) * 1000

        logger.debug(
            (
                "metrics global | cameras=%d healthy=%d "
                "avg_fps=%.2f avg_latency=%.2f | %.2f ms"
            ),
            camera_count,
            healthy_count,
            avg_fps,
            avg_latency,
            elapsed_ms,
        )

        return GlobalMetrics(
            camera_count=camera_count,
            healthy=healthy_count,
            unhealthy=unhealthy_count,
            avg_fps=avg_fps,
            avg_latency_ms=avg_latency
        )

    except Exception:
        logger.exception("metrics global unexpected error")
        raise HTTPException(
            status_code=500,
            detail="Internal server error"
        )