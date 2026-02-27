from fastapi import APIRouter, Request, UploadFile, File, HTTPException
from app.models import ParkingStats, RenderPlace, AffineTransform, RenderResponse
import numpy as np
import cv2
import time

router = APIRouter()

@router.get("/stats", response_model=ParkingStats)
def get_parking_stats(request: Request):
    logger = request.app.state.logger
    start_time = time.perf_counter()

    # Log endpoint call
    logger.info("GET /stats called")

    try:
        parking_ = request.app.state.parking_

        # Parking system not initialized yet
        if parking_ is None:
            logger.warning(
                "GET /stats failed: parking system not initialized"
            )
            raise HTTPException(
                status_code=503,
                detail="Parking system not initialized"
            )

        # Retrieve statistics from parking system
        stats = parking_.get_stats()

        elapsed_ms = (time.perf_counter() - start_time) * 1000

        # Log successful execution with statistics and timing
        logger.info(
            "GET /stats success | free=%s occupied=%s total=%s | %.2f ms",
            stats.get("free"),
            stats.get("occupied"),
            stats.get("total"),
            elapsed_ms,
        )

        return {
            "free_places": stats["free"],
            "occupied_places": stats["occupied"],
            "total_places": stats["total"],
        }

    except HTTPException:
        # Let FastAPI handle expected HTTP errors
        raise

    except Exception:
        # Catch any unexpected error and log full stack trace
        logger.exception(
            "GET /stats unexpected error"
        )
        raise HTTPException(
            status_code=500,
            detail="Internal server error"
        )

@router.get("/render", response_model=RenderResponse)
def get_render_data(request: Request):
    logger = request.app.state.logger
    start_time = time.perf_counter()

    # Log endpoint call
    logger.info("GET /render called")

    try:
        parking_ = request.app.state.parking_

        # Parking system not initialized yet
        if parking_ is None:
            logger.warning(
                "GET /render failed: parking system not initialized"
            )
            raise HTTPException(
                status_code=503,
                detail="Parking system not initialized"
            )

        # Retrieve render data from vision system
        coords, states, affine = parking_.get_render_data()

        # Safety check: ensure data consistency
        if len(coords) != len(states):
            logger.error(
                "GET /render data mismatch | coords=%d states=%d",
                len(coords),
                len(states),
            )
            raise HTTPException(
                status_code=500,
                detail="Render data corrupted"
            )

        # Convert numpy arrays to native Python lists
        coords = coords.tolist()    # (N, 4, 2)
        states = states.tolist()    # (N,)

        # Build response objects
        places = [
            RenderPlace(
                coords=coords[i],
                state=states[i],
            )
            for i in range(len(states))
        ]

        elapsed_ms = (time.perf_counter() - start_time) * 1000

        # Log successful execution with performance metrics
        logger.info(
            "GET /render success | places=%d | %.2f ms",
            len(places),
            elapsed_ms,
        )

        return RenderResponse(
            places=places,
            transform=AffineTransform(
                valid=affine["valid"],
                matrix=affine.get("matrix")
            )
        )

    except HTTPException:
        # Let FastAPI handle expected HTTP errors
        raise

    except Exception:
        # Catch any unexpected error and log full stack trace
        logger.exception(
            "GET /render unexpected error"
        )
        raise HTTPException(
            status_code=500,
            detail="Internal server error"
        )

@router.post("/process")
async def process_frame(
    request: Request,
    file: UploadFile = File(...)
):
    logger = request.app.state.logger
    start_time = time.perf_counter()

    # Log endpoint call
    logger.info(
        "POST /process called | filename=%s content_type=%s",
        file.filename,
        file.content_type,
    )

    try:
        parking_ = request.app.state.parking_

        # Parking system not initialized yet
        if parking_ is None:
            logger.warning(
                "POST /process failed: parking system not initialized"
            )
            raise HTTPException(
                status_code=503,
                detail="Parking system not initialized"
            )

        # Read uploaded image bytes
        data = await file.read()

        if not data:
            logger.warning(
                "POST /process failed: empty file received"
            )
            raise HTTPException(
                status_code=400,
                detail="Empty image file"
            )

        # Decode image using OpenCV
        np_img = np.frombuffer(data, np.uint8)
        frame = cv2.imdecode(np_img, cv2.IMREAD_COLOR)

        # Invalid or unsupported image format
        if frame is None:
            logger.warning(
                "POST /process failed: invalid image | filename=%s",
                file.filename,
            )
            raise HTTPException(
                status_code=400,
                detail="Invalid image"
            )

        h, w, c = frame.shape
        logger.debug(
            "POST /process image decoded | shape=%dx%dx%d",
            w, h, c,
        )

        # Run vision pipeline (detection / tracking / state update)
        parking_.evolve(frame)

        elapsed_ms = (time.perf_counter() - start_time) * 1000

        # Log successful processing with timing
        logger.info(
            "POST /process success | %.2f ms",
            elapsed_ms,
        )

        return {"status": "ok"}

    except HTTPException:
        # Expected errors are re-raised as-is
        raise

    except Exception:
        # Catch unexpected crashes (OpenCV, model inference, memory, etc.)
        logger.exception(
            "POST /process unexpected error"
        )
        raise HTTPException(
            status_code=500,
            detail="Internal server error"
        )