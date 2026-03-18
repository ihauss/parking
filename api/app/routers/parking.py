"""
Parking API.

Provides:
- Snapshot of parking state
- Business statistics
- Frame ingestion endpoint
"""

from fastapi import APIRouter, Request, HTTPException, UploadFile, File
import numpy as np
import cv2
import time

from app.models import (
    SnapshotResponse,
    RenderPlaceModel,
    AffineTransformModel,
    ParkingStatsResponse,
)

router = APIRouter()
MAX_SIZE = 5 * 1024 * 1024  # 5MB


# =========================
# GET Snapshot
# =========================

@router.get("/{camera_id}/snapshot", response_model=SnapshotResponse)
def get_snapshot(camera_id: str, request: Request):
    """
    Retrieve the current parking snapshot for a camera.
    """
    logger = request.app.state.logger
    start_time = time.perf_counter()

    parking_system = request.app.state.parking_system

    if parking_system is None:
        logger.error("Parking system not initialized")
        raise HTTPException(503, "Parking system not initialized")

    try:
        snapshot = parking_system.get_snapshot(camera_id)

        transform = (
            list(snapshot.affine)
            if snapshot.has_affine
            else None
        )

        elapsed_ms = (time.perf_counter() - start_time) * 1000

        logger.debug(
            "snapshot %s | occupied=%d | %.2f ms",
            camera_id,
            snapshot.num_occupied,
            elapsed_ms,
        )

        return SnapshotResponse(
            places=[
                RenderPlaceModel(
                    coords=place.coords,
                    state=place.state,
                )
                for place in snapshot.places
            ],
            num_occupied=snapshot.num_occupied,
            num_places=snapshot.num_places,
            transform=AffineTransformModel(
                valid=snapshot.has_affine,
                matrix=transform
            )
        )

    except RuntimeError as e:
        logger.warning("snapshot %s failed: %s", camera_id, str(e))
        raise HTTPException(404, str(e))

    except Exception:
        logger.exception("snapshot %s unexpected error", camera_id)
        raise HTTPException(500, "Internal server error")


# =========================
# GET Business Stats
# =========================

@router.get("/{camera_id}/stats", response_model=ParkingStatsResponse)
def get_parking_stats(camera_id: str, request: Request):
    """
    Retrieve business-level parking statistics.
    """
    logger = request.app.state.logger
    start_time = time.perf_counter()

    parking_system = request.app.state.parking_system

    if parking_system is None:
        logger.error("Parking system not initialized")
        raise HTTPException(503, "Parking system not initialized")

    try:
        stats = parking_system.get_stats(camera_id)

        elapsed_ms = (time.perf_counter() - start_time) * 1000

        logger.debug(
            "stats %s | occupied=%d | %.2f ms",
            camera_id,
            stats["occupied"],
            elapsed_ms,
        )

        return ParkingStatsResponse(
            occupied=int(stats["occupied"]),
            free=int(stats["free"]),
            total=int(stats["total"]),
        )

    except RuntimeError as e:
        logger.warning("stats %s failed: %s", camera_id, str(e))
        raise HTTPException(404, str(e))

    except Exception:
        logger.exception("stats %s unexpected error", camera_id)
        raise HTTPException(500, "Internal server error")


# =========================
# POST Frame
# =========================

@router.post("/{camera_id}/frame")
async def process_frame(
    camera_id: str,
    request: Request,
    file: UploadFile = File(...)
):
    """
    Receive and enqueue a frame for processing.
    """
    logger = request.app.state.logger
    start_time = time.perf_counter()

    parking_system = request.app.state.parking_system
    dispatcher = request.app.state.frame_dispatcher

    if parking_system is None:
        logger.error("Parking system not initialized")
        raise HTTPException(503, "Parking system not initialized")

    if dispatcher is None:
        logger.error("Frame dispatcher not initialized")
        raise HTTPException(503, "Frame dispatcher not initialized")

    try:
        contents = await file.read()

        if len(contents) > MAX_SIZE:
            raise HTTPException(400, "Image too large")

        # IMPORTANT: do NOT delete contents before using it
        np_array = np.frombuffer(contents, np.uint8)

        frame = cv2.imdecode(np_array, cv2.IMREAD_COLOR)

        if frame is None:
            raise HTTPException(400, "Invalid image")

        # Push frame (copy optional depending on thread safety)
        dispatcher.push_frame(camera_id, frame.copy())

        elapsed_ms = (time.perf_counter() - start_time) * 1000

        logger.debug(
            "frame %s received | %.2f ms",
            camera_id,
            elapsed_ms,
        )

        return {"detail": "Frame received"}

    except RuntimeError as e:
        logger.warning("frame %s failed: %s", camera_id, str(e))
        raise HTTPException(404, str(e))

    except HTTPException:
        raise

    except Exception:
        logger.exception("frame %s unexpected error", camera_id)
        raise HTTPException(500, "Internal server error")