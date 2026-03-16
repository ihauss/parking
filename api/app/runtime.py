"""
Global application state.

Hosts:
- Parking core instance (C++ binding)
- Initialization status
- Persistent camera reload at startup
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
    def __init__(self):
        self.initialized: bool = False

    # =========================
    # Initialization
    # =========================

    def initialize(self, app, start_watchdog=True):
        """
        Initialize core services.
        Called at FastAPI startup.
        """
        if self.initialized:
            return
        
        app.state.logger = setup_logger(level="INFO")

        app.state.logger.info("Initializing ParkingSystem...")

        parking_system = ParkingSystem()
        app.state.parking_system = parking_system

        app.state.watchdog = CameraWatchdog(
            app.state.parking_system,
            app.state.logger
        )

        app.state.frame_dispatcher = FrameDispatcher(parking_system)
        app.state.frame_dispatcher.start()

        if start_watchdog:
            try:
                loop = asyncio.get_running_loop()
                app.state.watchdog_task = loop.create_task(app.state.watchdog.run())
                app.state.logger.info("Watchdog started")
            except RuntimeError:
                app.state.logger.info("Watchdog not started (no running event loop)")
                app.state.watchdog_task = None
        else:
            app.state.logger.info("Watchdog disabled")
            app.state.watchdog_task = None

        # Ensure folders exist
        CAMERAS_DIR.mkdir(parents=True, exist_ok=True)

        # Reload cameras from disk
        self._reload_cameras(app)

        self.initialized = True

        app.state.logger.info("Application initialized successfully.")

    # =========================
    # Internal Reload Logic
    # =========================

    def _reload_cameras(self, app):
        """
        Reload all cameras found in data/cameras/.
        Skips invalid or corrupted folders.
        """
        assert app.state.parking_system is not None

        loaded = 0
        skipped = 0

        for camera_dir in CAMERAS_DIR.iterdir():

            if not camera_dir.is_dir():
                continue

            camera_id = camera_dir.name
            config_path = camera_dir / "config.json"
            reference_path = camera_dir / "reference.jpg"

            if not config_path.exists() or not reference_path.exists():
                app.state.logger.warning(
                    "Skipping incomplete camera folder: %s",
                    camera_id
                )
                skipped += 1
                continue

            try:
                reference = cv2.imread(str(reference_path))
                if reference is None:
                    raise ValueError("Invalid reference image")

                app.state.parking_system.add_camera(
                    camera_id,
                    str(config_path),
                    reference
                )

                app.state.logger.info(
                    "Camera reloaded successfully: %s",
                    camera_id
                )

                loaded += 1

            except Exception:
                app.state.logger.exception(
                    "Failed to reload camera: %s",
                    camera_id
                )
                skipped += 1

        app.state.logger.info(
            "Camera reload complete | loaded=%d | skipped=%d",
            loaded,
            skipped
        )


runtime = Runtime()