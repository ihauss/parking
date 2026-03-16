import pytest
import logging
import shutil
from pathlib import Path
import cv2
import numpy as np

from tests.system.fakers import FakeParkingSystem
from app.watchdog import CameraWatchdog

logger = logging.getLogger("test")

DATA_DIR = Path("data")
CAMERAS_DIR = DATA_DIR / "cameras"
CAM_DIR = CAMERAS_DIR / "cam1"


def delete_files():
    # nettoyage après test
    if CAM_DIR.exists():
        shutil.rmtree(CAM_DIR)


def setup_camera_files():
    # nettoyage avant test
    delete_files()

    CAM_DIR.mkdir(parents=True)

    # fichiers attendus par le watchdog
    (CAM_DIR / "config.json").write_text("{}")

    ref = np.zeros((10, 10, 3), dtype=np.uint8)
    cv2.imwrite(str(CAM_DIR / "reference.jpg"), ref)


@pytest.mark.anyio
async def test_watchdog_restart_unhealthy():

    fake_system = FakeParkingSystem()
    fake_system.add_camera("cam1")
    fake_system.cameras["cam1"]["healthy"] = False

    watchdog = CameraWatchdog(fake_system, logger=logger)

    setup_camera_files()

    # exécuter une iteration
    #await watchdog._iteration()
    watchdog._iteration()

    assert fake_system.cameras["cam1"]["restart_count"] == 1

    delete_files()


@pytest.mark.anyio
def test_watchdog_restart_error_state():

    fake_system = FakeParkingSystem()
    fake_system.add_camera("cam1")

    fake_system.cameras["cam1"]["state"] = "ERROR"
    fake_system.cameras["cam1"]["last_update"] = 3

    watchdog = CameraWatchdog(fake_system, logger=logger)

    setup_camera_files()

    watchdog._iteration()

    assert fake_system.cameras["cam1"]["restart_count"] == 1

    delete_files()


@pytest.mark.anyio
def test_watchdog_restart_frame_timeout():

    fake_system = FakeParkingSystem()
    fake_system.add_camera("cam1")

    fake_system.cameras["cam1"]["last_update"] = 10

    watchdog = CameraWatchdog(fake_system, logger=logger, timeout=5)

    setup_camera_files()

    watchdog._iteration()

    assert fake_system.cameras["cam1"]["restart_count"] == 1

    delete_files()


@pytest.mark.anyio
def test_watchdog_restart_high_latency():

    fake_system = FakeParkingSystem()
    fake_system.add_camera("cam1")

    fake_system.cameras["cam1"]["latency"] = 600
    fake_system.cameras["cam1"]["last_update"] = 2

    watchdog = CameraWatchdog(fake_system, logger=logger)

    setup_camera_files()

    watchdog._iteration()

    assert fake_system.cameras["cam1"]["restart_count"] == 1

    delete_files()


def test_watchdog_restart_cooldown():

    fake_system = FakeParkingSystem()
    fake_system.add_camera("cam1")

    fake_system.cameras["cam1"]["healthy"] = False

    watchdog = CameraWatchdog(fake_system, logger=logger)

    setup_camera_files()

    watchdog._iteration()
    watchdog._iteration()

    assert fake_system.cameras["cam1"]["restart_count"] == 1

    delete_files()


@pytest.mark.anyio
def test_watchdog_missing_files():

    fake_system = FakeParkingSystem()
    fake_system.add_camera("cam1")

    watchdog = CameraWatchdog(fake_system, logger=logger)

    setup_camera_files()

    watchdog._iteration()

    assert fake_system.cameras["cam1"]["restart_count"] == 0

    setup_camera_files()


@pytest.mark.anyio
def test_watchdog_missing_files():

    fake_system = FakeParkingSystem()
    fake_system.add_camera("cam1")

    watchdog = CameraWatchdog(fake_system, logger=logger)

    if CAM_DIR.exists():
        shutil.rmtree(CAM_DIR)

    watchdog._iteration()

    assert fake_system.cameras["cam1"]["restart_count"] == 0

    setup_camera_files()


@pytest.mark.anyio
def test_watchdog_invalid_reference():

    fake_system = FakeParkingSystem()
    fake_system.add_camera("cam1")

    watchdog = CameraWatchdog(fake_system, logger=logger)

    delete_files()

    CAM_DIR.mkdir(parents=True)

    (CAM_DIR / "config.json").write_text("{}")

    (CAM_DIR / "reference.jpg").write_text("not an image")

    watchdog._iteration()

    assert fake_system.cameras["cam1"]["restart_count"] == 0

    setup_camera_files()


@pytest.mark.anyio
def test_watchdog_reference_cache_cleanup():

    fake_system = FakeParkingSystem()
    fake_system.add_camera("cam1")

    watchdog = CameraWatchdog(fake_system, logger=logger)

    setup_camera_files()

    watchdog._iteration()

    assert "cam1" in watchdog.references

    fake_system.cameras = {}

    watchdog._iteration()

    assert "cam1" not in watchdog.references