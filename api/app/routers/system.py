from fastapi import APIRouter, Request, HTTPException, UploadFile, File, Form
from pydantic import BaseModel
from pathlib import Path
import shutil
import json
import re
import cv2
import numpy as np
import time
from app.models import CameraInfo, HealthResponse, CameraStateResponse

router = APIRouter()

# =========================
# Constants
# =========================

DATA_DIR = Path("data")
CAMERAS_DIR = DATA_DIR / "cameras"

ID_REGEX = re.compile(r"^[a-zA-Z0-9_-]+$")


# =========================
# Utility Functions
# =========================

def ensure_directories():
    CAMERAS_DIR.mkdir(parents=True, exist_ok=True)


def validate_camera_id(camera_id: str):
    if not ID_REGEX.match(camera_id):
        raise HTTPException(
            status_code=400,
            detail="Invalid camera_id format"
        )


def load_reference_image(path: Path):
    img = cv2.imread(str(path))
    if img is None:
        raise ValueError("Invalid reference image")
    return img


# =========================
# Startup Reload Helper
# =========================

def reload_cameras_from_disk(app):
    logger = app.state.logger
    parking_system = app.state.parking_system

    ensure_directories()

    for camera_folder in CAMERAS_DIR.iterdir():
        if not camera_folder.is_dir():
            continue

        camera_id = camera_folder.name
        config_path = camera_folder / "config.json"
        reference_path = camera_folder / "reference.jpg"

        if not config_path.exists() or not reference_path.exists():
            logger.warning(
                "Skipping incomplete camera folder: %s",
                camera_id
            )
            continue

        try:
            reference = load_reference_image(reference_path)

            parking_system.add_camera(
                camera_id,
                str(config_path),
                reference
            )

            logger.info("Loaded camera from disk: %s", camera_id)

        except Exception:
            logger.exception(
                "Failed to load camera %s at startup",
                camera_id
            )


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
    logger = request.app.state.logger
    parking_system = request.app.state.parking_system
    frame_dispatcher = request.app.state.frame_dispatcher
    start_time = time.perf_counter()

    validate_camera_id(camera_id)

    ensure_directories()

    camera_dir = CAMERAS_DIR / camera_id

    if camera_dir.exists():
        raise HTTPException(
            status_code=409,
            detail="Camera already exists"
        )

    try:
        # Create directory
        camera_dir.mkdir(parents=True)

        # Save config
        config_path = camera_dir / "config.json"
        config_bytes = await config.read()

        try:
            json.loads(config_bytes.decode())
        except Exception:
            raise HTTPException(
                status_code=400,
                detail="Invalid JSON configuration"
            )

        with open(config_path, "wb") as f:
            f.write(config_bytes)

        # Save reference
        reference_path = camera_dir / "reference.jpg"
        reference_bytes = await reference.read()

        with open(reference_path, "wb") as f:
            f.write(reference_bytes)

        # Validate reference image
        reference_img = load_reference_image(reference_path)

        # Add to C++ system
        parking_system.add_camera(
            camera_id,
            str(config_path),
            reference_img
        )

        frame_dispatcher.add_camera(camera_id)

        elapsed_ms = (time.perf_counter() - start_time) * 1000
        logger.info(
            "Camera %s added successfully | %.2f ms",
            camera_id,
            elapsed_ms
        )

        return {"detail": "Camera created"}

    except HTTPException:
        shutil.rmtree(camera_dir, ignore_errors=True)
        raise

    except Exception:
        logger.exception("Failed to create camera %s", camera_id)
        shutil.rmtree(camera_dir, ignore_errors=True)
        raise HTTPException(
            status_code=500,
            detail="Internal server error"
        )


# =========================
# DELETE - Remove Camera
# =========================

@router.delete("/cameras/{camera_id}", status_code=204)
def remove_camera(camera_id: str, request: Request):
    logger = request.app.state.logger
    parking_system = request.app.state.parking_system
    frame_dispatcher = request.app.state.frame_dispatcher

    validate_camera_id(camera_id)

    camera_dir = CAMERAS_DIR / camera_id

    try:
        parking_system.remove_camera(camera_id)
        frame_dispatcher.remove_camera(camera_id)

    except RuntimeError:
        raise HTTPException(status_code=404, detail="Camera not found")

    except Exception:
        logger.exception("Unexpected error removing camera")
        raise HTTPException(status_code=500, detail="Internal server error")

    # Remove directory
    shutil.rmtree(camera_dir, ignore_errors=True)

    logger.info("Camera %s removed", camera_id)


# =========================
# POST - Restart Camera
# =========================

@router.post("/cameras/{camera_id}/restart")
def restart_camera(camera_id: str, request: Request):
    logger = request.app.state.logger
    parking_system = request.app.state.parking_system

    validate_camera_id(camera_id)

    camera_dir = CAMERAS_DIR / camera_id
    config_path = camera_dir / "config.json"
    reference_path = camera_dir / "reference.jpg"

    if not camera_dir.exists():
        raise HTTPException(status_code=404, detail="Camera not found")

    try:
        reference_img = load_reference_image(reference_path)

        parking_system.restart_camera(
            camera_id,
            str(config_path),
            reference_img
        )

        logger.info("Camera %s restarted", camera_id)

        return {"detail": "Camera restarted"}

    except RuntimeError:
        raise HTTPException(status_code=404, detail="Camera not found")

    except Exception:
        logger.exception("Unexpected error restarting camera")
        raise HTTPException(status_code=500, detail="Internal server error")


# =========================
# GET - List Cameras
# =========================

@router.get("/cameras", response_model=list[CameraInfo])
def list_cameras(request: Request):
    parking_system = request.app.state.parking_system

    ids = parking_system.list_cameras()

    return [
        CameraInfo(
            id=camera_id,
            healthy=parking_system.is_healthy(camera_id)
        )
        for camera_id in ids
    ]


# =========================
# GET - Camera Health
# =========================

@router.get("/cameras/{camera_id}/health", response_model=HealthResponse)
def camera_health(camera_id: str, request: Request):
    parking_system = request.app.state.parking_system

    try:
        healthy = parking_system.is_healthy(camera_id)
        return HealthResponse(healthy=healthy)

    except RuntimeError:
        raise HTTPException(status_code=404, detail="Camera not found")
    
@router.get("/cameras/{camera_id}/state", response_model=CameraStateResponse)
def state_str(camera_id: str, request: Request):
    parking_system = request.app.state.parking_system

    try:
        state_str = parking_system.get_cam_state_str(camera_id)
        healthy = parking_system.is_healthy(camera_id)
        return CameraStateResponse(state=state_str, healthy=healthy)

    except RuntimeError:
        raise HTTPException(status_code=404, detail="Camera not found")