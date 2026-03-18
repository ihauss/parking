from fastapi import FastAPI
from contextlib import asynccontextmanager
import logging

from app.routers import parking, metrics, system
from app.logger import setup_logger
from app.runtime import runtime


@asynccontextmanager
async def lifespan(app: FastAPI):
    """
    Application lifecycle manager.

    Handles:
    - Startup initialization of core services
    - Graceful shutdown of background components

    Ensures the ParkingSystem and related services are properly
    initialized before serving requests and cleanly stopped on exit.
    """

    # =========================
    # Startup
    # =========================

    logger = setup_logger(level=logging.INFO)
    logger.info("Application starting")

    try:
        # Initialize runtime (ParkingSystem, dispatcher, etc.)
        runtime.initialize(app)

        logger.info("Parking system initialized (no cameras loaded)")

    except Exception:
        logger.exception("Failed to initialize parking system")
        runtime.initialized = False
        raise

    yield

    # =========================
    # Shutdown
    # =========================

    logger.info("Application shutting down")

    try:
        # Stop watchdog safely if present
        watchdog = getattr(app.state, "watchdog", None)
        watchdog_task = getattr(app.state, "watchdog_task", None)

        if watchdog is not None:
            watchdog.running = False

        if watchdog_task is not None:
            await watchdog_task

        # Cleanup core system
        app.state.parking_system = None
        runtime.initialized = False

        logger.info("Shutdown completed")

    except Exception:
        logger.exception("Error during shutdown")


app = FastAPI(
    title="Smart Parking API",
    version="0.1.0",
    description="Push-based multi-camera parking API",
    lifespan=lifespan
)

app.include_router(parking.router, prefix="/parking", tags=["parking"])
app.include_router(metrics.router, prefix="/metrics", tags=["metrics"])
app.include_router(system.router, prefix="/system", tags=["system"])