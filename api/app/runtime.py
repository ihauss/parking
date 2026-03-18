"""
Global application runtime state.

Responsibilities:
- Manage initialization of core services (ParkingSystem, dispatcher, watchdog)
- Maintain application-wide state
- Reload persisted cameras at startup
"""

from pathlib import Path
import logging
import cv2
import asyncio

from app.logger import setup_logger
from app.watchdog import CameraWatchdog
from app.FrameDispatcher import FrameDispatcher

from smart_parking_core import ParkingSystem


DATA_DIR = Path("data")
CAMERAS_DIR = DATA_DIR / "cameras"


class Runtime:
    """
    Central runtime manager for the application.

    Handles:
    - Initialization of core components
    - Lifecycle state (initialized / not initialized)
    - Camera reloading from disk
    """

    def __init__(self):
        self.initialized: bool = False

    # =========================
    # Initialization
    # =========================

    def initialize(self, app, start_watchdog: bool = True):
        """
        Initialize all core services.

        This method is called at FastAPI startup and ensures that:
        - ParkingSystem is created
        - FrameDispatcher is started
        - Watchdog is optionally started
        - Cameras are reloaded from disk

        Args:
            app: FastAPI application instance
            start_watchdog (bool): Whether to start the watchdog task
        """
        if self.initialized:
            return

        # Setup logger
        app.state.logger = setup_logger(level=logging.INFO)
        logger = app.state.logger

        logger.info("Initializing parking system...")

        # Initialize core system
        parking_system = ParkingSystem()
        app.state.parking_system = parking_system

        # Initialize dispatcher (frame processing)
        app.state.frame_dispatcher = FrameDispatcher(parking_system)
        app.state.frame_dispatcher.start()

        # Initialize watchdog (camera health monitoring)
        app.state.watchdog = CameraWatchdog(
            parking_system,
            logger
        )

        if start_watchdog:
            try:
                loop = asyncio.get_running_loop()
                app.state.watchdog_task = loop.create_task(
                    app.state.watchdog.run()
                )
                logger.info("Watchdog started")

            except RuntimeError:
                logger.warning(
                    "Watchdog not started (no running event loop)"
                )
                app.state.watchdog_task = None
        else:
            logger.info("Watchdog disabled")
            app.state.watchdog_task = None

        # Ensure required directories exist
        CAMERAS_DIR.mkdir(parents=True, exist_ok=True)

        # Reload cameras from disk
        self._reload_cameras(app)

        self.initialized = True

        logger.info("Application initialized successfully")

    # =========================
    # Internal Reload Logic
    # =========================

    def _reload_cameras(self, app):
        """
        Reload all persisted cameras from disk.

        Expected structure:
            data/cameras/<camera_id>/
                - config.json
                - reference.jpg

        Invalid or incomplete camera folders are skipped.

        Args:
            app: FastAPI application instance
        """
        parking_system = app.state.parking_system
        logger = app.state.logger

        assert parking_system is not None

        loaded = 0
        skipped = 0

        for camera_dir in CAMERAS_DIR.iterdir():

            if not camera_dir.is_dir():
                continue

            camera_id = camera_dir.name
            config_path = camera_dir / "config.json"
            reference_path = camera_dir / "reference.jpg"

            # Validate required files
            if not config_path.exists() or not reference_path.exists():
                logger.warning(
                    "Skipping incomplete camera folder: %s",
                    camera_id
                )
                skipped += 1
                continue

            try:
                # Load reference image
                reference = cv2.imread(str(reference_path))
                if reference is None:
                    raise ValueError("Invalid reference image")

                # Register camera in core system
                parking_system.add_camera(
                    camera_id,
                    str(config_path),
                    reference
                )

                logger.info(
                    "Camera reloaded successfully: %s",
                    camera_id
                )

                loaded += 1

            except Exception:
                logger.exception(
                    "Failed to reload camera: %s",
                    camera_id
                )
                skipped += 1

        logger.info(
            "Camera reload complete | loaded=%d | skipped=%d",
            loaded,
            skipped
        )


runtime = Runtime()