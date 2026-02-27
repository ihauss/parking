from fastapi import APIRouter, Request, HTTPException
from app.models import ParkingMetrics

router = APIRouter()


import time
from fastapi import HTTPException, Request

@router.get("/", response_model=ParkingMetrics)
def get_metrics(request: Request):
    logger = request.app.state.logger
    start_time = time.perf_counter()

    # Log endpoint call
    logger.info("GET /metrics called")

    try:
        parking_ = request.app.state.parking_

        # Parking system not initialized yet
        if parking_ is None:
            logger.warning(
                "GET /metrics failed: parking system not initialized"
            )
            raise HTTPException(
                status_code=503,
                detail="Parking system not initialized"
            )

        # Retrieve metrics from parking system
        stats = parking_.get_stats()

        elapsed_ms = (time.perf_counter() - start_time) * 1000

        # Log successful execution with metrics and timing
        logger.info(
            "GET /metrics success | fps=%.2f latency_ms=%.2f | %.2f ms",
            stats.get("fps"),
            stats.get("latency_ms"),
            elapsed_ms,
        )

        return {
            "fps": stats["fps"],
            "last_latency_ms": stats["latency_ms"],
        }

    except HTTPException:
        # Let FastAPI handle expected HTTP errors
        raise

    except Exception:
        # Catch any unexpected error and log full stack trace
        logger.exception(
            "GET /metrics unexpected error"
        )
        raise HTTPException(
            status_code=500,
            detail="Internal server error"
        )
