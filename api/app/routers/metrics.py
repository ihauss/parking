from fastapi import APIRouter, Request, HTTPException
from pydantic import BaseModel
from typing import List
import time
from app.models import CameraMetrics, GlobalMetrics

router = APIRouter()


# =========================
# Per-camera metrics
# =========================

@router.get("/{camera_id}", response_model=CameraMetrics)
def get_camera_metrics(camera_id: str, request: Request):
    logger = request.app.state.logger
    start_time = time.perf_counter()

    logger.info("GET /metrics/%s called", camera_id)

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

        logger.info(
            "GET /metrics/%s success | fps=%.2f latency=%.2f | %.2f ms",
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
            "GET /metrics/%s failed: %s",
            camera_id,
            str(e)
        )
        raise HTTPException(
            status_code=404,
            detail=str(e)
        )

    except Exception:
        logger.exception(
            "GET /metrics/%s unexpected error",
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
    logger = request.app.state.logger
    start_time = time.perf_counter()

    logger.info("GET /metrics called")

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
            logger.info("No cameras registered")

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

        for camera_id in camera_ids:
            try:
                stats = parking_system.get_stats(camera_id)
                fps_values.append(stats["fps"])
                latency_values.append(stats["latency_ms"])

                if parking_system.is_healthy(camera_id):
                    healthy_count += 1

            except RuntimeError:
                # Camera may have been removed mid-iteration
                logger.warning(
                    "Camera %s disappeared during aggregation",
                    camera_id
                )
                continue

        camera_count = len(camera_ids)
        unhealthy_count = camera_count - healthy_count

        avg_fps = (
            sum(fps_values) / len(fps_values)
            if fps_values else 0.0
        )

        avg_latency = (
            sum(latency_values) / len(latency_values)
            if latency_values else 0.0
        )

        elapsed_ms = (time.perf_counter() - start_time) * 1000

        logger.info(
            "GET /metrics success | cameras=%d healthy=%d avg_fps=%.2f | %.2f ms",
            camera_count,
            healthy_count,
            avg_fps,
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
        logger.exception("GET /metrics unexpected error")
        raise HTTPException(
            status_code=500,
            detail="Internal server error"
        )