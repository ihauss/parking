from fastapi import FastAPI
from contextlib import asynccontextmanager

from app.routers import parking, metrics, system
from app.logger import setup_logger
from app.runtime import runtime


from smart_parking_core import ParkingSystem


@asynccontextmanager
async def lifespan(app: FastAPI):

    # =========================
    # Startup
    # =========================

    logger = setup_logger(level="INFO")
    logger.info("Application starting")

    try:
        # Instantiate core system (no cameras yet)
        runtime.initialize(app)
        logger.info("ParkingSystem initialized")

        logger.info("ParkingSystem initialized (no cameras loaded)")

    except Exception as e:
        logger.exception("Failed to initialize ParkingSystem")
        runtime.initialized = False
        raise e

    yield

    # =========================
    # Shutdown
    # =========================

    logger.info("Application shutting down")

    app.state.watchdog.running = False
    await app.state.watchdog_task
    app.state.parking_system = None
    runtime.initialized = False


app = FastAPI(
    title="Smart Parking API",
    version="0.1.0",
    description="Push-based multi-camera parking API",
    lifespan=lifespan
)

app.include_router(parking.router, prefix="/parking", tags=["parking"])
app.include_router(metrics.router, prefix="/metrics", tags=["metrics"])
app.include_router(system.router, prefix="/system", tags=["system"])