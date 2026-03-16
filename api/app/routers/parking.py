from fastapi import APIRouter, Request, HTTPException, UploadFile, File
from pydantic import BaseModel
import numpy as np
import cv2
import time
from app.models import SnapshotResponse, RenderPlaceModel, AffineTransformModel, ParkingStatsResponse

router = APIRouter()

# =========================
# GET Snapshot
# =========================

@router.get("/{camera_id}/snapshot", response_model=SnapshotResponse)
def get_snapshot(camera_id: str, request: Request):
    logger = request.app.state.logger
    start_time = time.perf_counter()

    logger.info("GET /parking/%s/snapshot called", camera_id)

    parking_system = request.app.state.parking_system

    if parking_system is None:
        logger.error("Parking system not initialized")
        raise HTTPException(status_code=503, detail="Parking system not initialized")

    try:
        snapshot = parking_system.get_snapshot(camera_id)

        transform = None
        if snapshot.has_affine:
            transform = list(snapshot.affine)

        elapsed_ms = (time.perf_counter() - start_time) * 1000

        logger.info(
            "GET /parking/%s/snapshot success | occupied=%d | %.2f ms",
            camera_id,
            snapshot.num_occupied,
            elapsed_ms,
        )

        return SnapshotResponse(
            places=[
                RenderPlaceModel(
                    coords=[(x, y) for (x, y) in place.coords],
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
        logger.warning(
            "GET /parking/%s/snapshot failed: %s",
            camera_id,
            str(e)
        )
        raise HTTPException(status_code=404, detail=str(e))

    except Exception:
        logger.exception("Unexpected error in snapshot")
        raise HTTPException(status_code=500, detail="Internal server error")


# =========================
# GET Business Stats
# =========================

@router.get("/{camera_id}/stats", response_model=ParkingStatsResponse)
def get_parking_stats(camera_id: str, request: Request):
    logger = request.app.state.logger
    start_time = time.perf_counter()

    logger.info("GET /parking/%s/stats called", camera_id)

    parking_system = request.app.state.parking_system

    if parking_system is None:
        raise HTTPException(status_code=503, detail="Parking system not initialized")

    try:
        stats = parking_system.get_stats(camera_id)

        elapsed_ms = (time.perf_counter() - start_time) * 1000

        logger.info(
            "GET /parking/%s/stats success | occupied=%d | %.2f ms",
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
        logger.warning("Parking stats failed: %s", str(e))
        raise HTTPException(status_code=404, detail=str(e))

    except Exception:
        logger.exception("Unexpected error in parking stats")
        raise HTTPException(status_code=500, detail="Internal server error")


# =========================
# POST Frame
# =========================

@router.post("/{camera_id}/frame")
async def process_frame(camera_id: str, request: Request, file: UploadFile = File(...)):

    logger = request.app.state.logger
    start_time = time.perf_counter()

    logger.info("POST /parking/%s/frame called", camera_id)

    parking_system = request.app.state.parking_system
    dispatcher = request.app.state.frame_dispatcher

    if parking_system is None:
        raise HTTPException(status_code=503, detail="Parking system not initialized")

    if dispatcher is None:
        raise HTTPException(status_code=503, detail="Frame dispatcher not initialized")

    try:
        contents = await file.read()

        np_array = np.frombuffer(contents, np.uint8)
        frame = cv2.imdecode(np_array, cv2.IMREAD_COLOR)

        if frame is None:
            raise HTTPException(status_code=400, detail="Invalid image")

        dispatcher.push_frame(camera_id, frame)

        elapsed_ms = (time.perf_counter() - start_time) * 1000

        logger.info(
            "POST /parking/%s/frame received | %.2f ms",
            camera_id,
            elapsed_ms,
        )

        return {"detail": "Frame received"}

    except RuntimeError as e:
        logger.warning("Process frame failed: %s", str(e))
        raise HTTPException(status_code=404, detail=str(e))

    except HTTPException:
        raise

    except Exception:
        logger.exception("Unexpected error in process_frame")
        raise HTTPException(status_code=500, detail="Internal server error")