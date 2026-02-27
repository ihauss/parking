from fastapi import FastAPI

from app.routers import parking, metrics, system

# app/main.py
from fastapi import FastAPI
from contextlib import asynccontextmanager
import cv2
from typing import Optional
from app.logger import setup_logger

@asynccontextmanager
async def lifespan(app: FastAPI):
    # === Startup ===
    from smart_parking_core import Parking
    app.state.parking_: Optional[Parking] = None
    logger = setup_logger(level="INFO")
    app.state.logger = logger

    logger.info("Application starting")
    reference = cv2.imread("data/reference.jpg")

    app.state.parking_ = Parking(
        "data/parking_config.json",
        reference
    )
    logger.info("Parking system initialized")

    yield
    # === Shutdown ===
    app.state.parking_ = None

app = FastAPI(
    title="Smart Parking API",
    version="0.1.0",
    description="API for parking occupancy estimation",
    lifespan=lifespan
)

app.include_router(parking.router, prefix="/parking", tags=["parking"])
app.include_router(metrics.router, prefix="/metrics", tags=["metrics"])
app.include_router(system.router, prefix="/system", tags=["system"])
