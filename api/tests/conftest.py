# api/tests/conftest.py

import pytest
import logging
import numpy as np
import tempfile
import shutil
from fastapi.testclient import TestClient
import sys
from pathlib import Path

from smart_parking_core import ParkingSystem

ROOT = Path(__file__).resolve().parents[1]

sys.path.append(str(ROOT))

from app.main import app


# -------------------------
# Fake Parking (unit tests)
# -------------------------

import numpy as np


class FakeParking:
    def get_stats(self):
        return {
            "free": 3,
            "occupied": 2,
            "total": 5,
            "fps": 27.5,
            "latency_ms": 18.3,
        }

    def get_render_data(self):
        coords = np.array([
            [[0, 0], [10, 0], [10, 10], [0, 10]],
        ])
        states = np.array([1])

        affine = {
            "valid": True,
            "matrix": [[1, 0, 0], [0, 1, 0]],
        }

        return coords, states, affine


class FakeParkingSystem:

    def __init__(self):
        self.cameras = {}

    # -------------------------
    # Camera management
    # -------------------------

    def add_camera(self, cam_id, config_path=None, reference=None):
        self.cameras[cam_id] = FakeParking()

    def remove_camera(self, cam_id):
        self.cameras.pop(cam_id, None)

    def list_cameras(self):
        return list(self.cameras.keys())

    # -------------------------
    # Camera state
    # -------------------------

    def get_cam_state_str(self, cam_id):
        return "RUNNING"

    def is_healthy(self, cam_id):
        return True

    def get_last_update_seconds(self, cam_id):
        return 0.1

    # -------------------------
    # Parking data
    # -------------------------

    def get_stats(self, cam_id):
        return self.cameras[cam_id].get_stats()

    def get_render_data(self, cam_id):
        return self.cameras[cam_id].get_render_data()

    # -------------------------
    # Watchdog compatibility
    # -------------------------

    def restart_camera(self, cam_id, config_path, reference):
        # simulate restart
        self.cameras[cam_id] = FakeParking()

    def evolve(self, frame):
        pass

    def process_frame(self, cid, frame):
        pass

class FakeFrameDispacher:
    def __init__(self, system):
        self.system = system

    def add_camera(self, camera_id):
        return

    def remove_camera(self, camera_id):
        return

    def push_frame(self, camera_id, frame):
        self.system.process_frame(camera_id, frame)


# -------------------------
# UNIT TEST CLIENT
# -------------------------

@pytest.fixture
def client_unit():
    """
    API client with FakeParking
    Used for unit tests of routes.
    """
    app.state.parking_system = FakeParkingSystem()
    app.state.logger = logging.getLogger("test")
    app.state.frame_dispatcher = FakeFrameDispacher(app.state.parking_system)

    return TestClient(app)


@pytest.fixture
def client_no_parking():
    """
    Simulates API when parking system not initialized.
    """
    app.state.parking_ = None
    return TestClient(app)


# -------------------------
# INTEGRATION TEST CLIENT
# -------------------------

@pytest.fixture
def client_integration(tmp_path):

    from app.runtime import runtime

    # dossier data temporaire
    data_dir = tmp_path / "data"
    cameras_dir = data_dir / "cameras"
    cameras_dir.mkdir(parents=True)

    # reset état
    runtime.initialized = False
    runtime.parking_system = None
    runtime.logger = logging.getLogger("test_logger")

    # initialisation normale (comme en prod)
    runtime.initialize(app, start_watchdog=False)

    client = TestClient(app)

    yield client

    # cleanup
    shutil.rmtree(data_dir, ignore_errors=True)

@pytest.fixture
def client_watchdog(tmp_path):

    from fastapi.testclient import TestClient
    from app.main import app
    from app.runtime import runtime

    runtime.initialized = False

    runtime.initialize(app, start_watchdog=True)

    client = TestClient(app)

    yield client

    if app.state.watchdog_task:
        app.state.watchdog.running = False