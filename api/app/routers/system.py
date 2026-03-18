"""
Camera management API.

Provides:
- Camera lifecycle management (add, remove, restart)
- Health and state monitoring
- Persistent storage on disk
"""

from fastapi import APIRouter, Request, HTTPException, UploadFile, File, Form
from pathlib import Path
import shutil
import json
import re
import cv2
import numpy as np
import time

from app.models import CameraInfo, HealthResponse, CameraStateResponse


router = APIRouter()

MAX_CONFIG_SIZE = 1 * 1024 * 1024
MAX_IMAGE_SIZE = 5 * 1024 * 1024


# =========================
# Constants
# =========================

DATA_DIR = Path("data")
CAMERAS_DIR = DATA_DIR / "cameras"

ID_REGEX = re.compile(r"^[a-zA-Z0-9_-]+$")


# =========================
# Utilities
# =========================

def ensure_directories():
    CAMERAS_DIR.mkdir(parents=True, exist_ok=True)


def validate_camera_id(camera_id: str):
    if not ID_REGEX.match(camera_id):
        raise HTTPException(400, "Invalid camera_id format")


def load_reference_image(path: Path):
    img = cv2.imread(str(path))
    if img is None:
        raise ValueError("Invalid reference image")
    return img


# =========================
# POST - Add Camera
# =========================

@router.post("/cameras", status_code=201)
async def add_camera(
    request: Request,
    camera_id: str = Form(...),
    config: UploadFile = File(...),
    reference: UploadFile = File(...)
):
    """
    Register a new camera with configuration and reference image.
    """
    logger = request.app.state.logger
    parking_system = request.app.state.parking_system
    dispatcher = request.app.state.frame_dispatcher
    start_time = time.perf_counter()

    if parking_system is None:
        logger.error("Parking system not initialized")
        raise HTTPException(503, "Parking system not initialized")

    if dispatcher is None:
        logger.error("Frame dispatcher not initialized")
        raise HTTPException(503, "Frame dispatcher not initialized")

    validate_camera_id(camera_id)
    ensure_directories()

    camera_dir = CAMERAS_DIR / camera_id

    if camera_dir.exists():
        raise HTTPException(409, "Camera already exists")

    try:
        camera_dir.mkdir(parents=True)

        # =========================
        # Config
        # =========================

        config_bytes = await config.read()

        if len(config_bytes) > MAX_CONFIG_SIZE:
            raise HTTPException(400, "Config too large")

        try:
            json.loads(config_bytes.decode())
        except Exception:
            raise HTTPException(400, "Invalid JSON configuration")

        config_path = camera_dir / "config.json"
        config_path.write_bytes(config_bytes)

        # =========================
        # Reference image
        # =========================

        reference_bytes = await reference.read()

        if len(reference_bytes) > MAX_IMAGE_SIZE:
            raise HTTPException(400, "Reference image too large")

        reference_path = camera_dir / "reference.jpg"
        reference_path.write_bytes(reference_bytes)

        reference_img = load_reference_image(reference_path)

        # =========================
        # Register camera
        # =========================

        parking_system.add_camera(
            camera_id,
            str(config_path),
            reference_img
        )

        dispatcher.add_camera(camera_id)

        elapsed_ms = (time.perf_counter() - start_time) * 1000

        logger.info(
            "camera %s created | %.2f ms",
            camera_id,
            elapsed_ms
        )

        return {"detail": "Camera created"}

    except HTTPException:
        shutil.rmtree(camera_dir, ignore_errors=True)
        raise

    except Exception:
        logger.exception("camera %s creation failed", camera_id)
        shutil.rmtree(camera_dir, ignore_errors=True)
        raise HTTPException(500, "Internal server error")


# =========================
# DELETE - Remove Camera
# =========================

@router.delete("/cameras/{camera_id}", status_code=204)
def remove_camera(camera_id: str, request: Request):
    """
    Remove a camera and its associated data.
    """
    logger = request.app.state.logger
    parking_system = request.app.state.parking_system
    dispatcher = request.app.state.frame_dispatcher

    if parking_system is None:
        logger.error("Parking system not initialized")
        raise HTTPException(503, "Parking system not initialized")

    if dispatcher is None:
        logger.error("Frame dispatcher not initialized")
        raise HTTPException(503, "Frame dispatcher not initialized")

    validate_camera_id(camera_id)

    camera_dir = CAMERAS_DIR / camera_id

    try:
        parking_system.remove_camera(camera_id)
        dispatcher.remove_camera(camera_id)

    except RuntimeError:
        raise HTTPException(404, "Camera not found")

    except Exception:
        logger.exception("camera %s removal failed", camera_id)
        raise HTTPException(500, "Internal server error")

    shutil.rmtree(camera_dir, ignore_errors=True)

    logger.info("camera %s removed", camera_id)


# =========================
# POST - Restart Camera
# =========================

@router.post("/cameras/{camera_id}/restart")
def restart_camera(camera_id: str, request: Request):
    """
    Restart a camera using persisted configuration.
    """
    logger = request.app.state.logger
    parking_system = request.app.state.parking_system

    if parking_system is None:
        logger.error("Parking system not initialized")
        raise HTTPException(503, "Parking system not initialized")

    validate_camera_id(camera_id)

    camera_dir = CAMERAS_DIR / camera_id

    if not camera_dir.exists():
        raise HTTPException(404, "Camera not found")

    config_path = camera_dir / "config.json"
    reference_path = camera_dir / "reference.jpg"

    try:
        reference_img = load_reference_image(reference_path)

        parking_system.restart_camera(
            camera_id,
            str(config_path),
            reference_img
        )

        logger.info("camera %s restarted", camera_id)

        return {"detail": "Camera restarted"}

    except RuntimeError:
        raise HTTPException(404, "Camera not found")

    except Exception:
        logger.exception("camera %s restart failed", camera_id)
        raise HTTPException(500, "Internal server error")


# =========================
# GET - List Cameras
# =========================

@router.get("/cameras", response_model=list[CameraInfo])
def list_cameras(request: Request):
    """
    List all registered cameras.
    """
    parking_system = request.app.state.parking_system
    logger = request.app.state.logger

    if parking_system is None:
        logger.error("Parking system not initialized")
        raise HTTPException(503, "Parking system not initialized")

    try:
        ids = parking_system.list_cameras()

        return [
            CameraInfo(
                id=camera_id,
                healthy=parking_system.is_healthy(camera_id)
            )
            for camera_id in ids
        ]

    except Exception:
        logger.exception("list cameras failed")
        raise HTTPException(500, "Internal server error")


# =========================
# GET - Camera Health
# =========================

@router.get("/cameras/{camera_id}/health", response_model=HealthResponse)
def camera_health(camera_id: str, request: Request):
    """
    Get camera health status.
    """
    parking_system = request.app.state.parking_system
    logger = request.app.state.logger

    if parking_system is None:
        logger.error("Parking system not initialized")
        raise HTTPException(503, "Parking system not initialized")

    try:
        return HealthResponse(
            healthy=parking_system.is_healthy(camera_id)
        )

    except RuntimeError:
        raise HTTPException(404, "Camera not found")


# =========================
# GET - Camera State
# =========================

@router.get("/cameras/{camera_id}/state", response_model=CameraStateResponse)
def camera_state(camera_id: str, request: Request):
    """
    Get camera runtime state.
    """
    parking_system = request.app.state.parking_system
    logger = request.app.state.logger

    if parking_system is None:
        logger.error("Parking system not initialized")
        raise HTTPException(503, "Parking system not initialized")

    try:
        return CameraStateResponse(
            state=parking_system.get_cam_state_str(camera_id),
            healthy=parking_system.is_healthy(camera_id)
        )

    except RuntimeError:
        raise HTTPException(404, "Camera not found")