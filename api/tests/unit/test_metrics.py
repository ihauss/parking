import io
import numpy as np
import cv2
from tests.unit.helpers import create_test_image, create_invalid_image
from tests.unit.fakers import FakeSnapshot

# =========================
# SNAPSHOT
# =========================

def test_snapshot_success(client_unit):

    system = client_unit.app.state.parking_system
    system.add_camera("cam1")

    system.get_snapshot = lambda cid: FakeSnapshot()

    r = client_unit.get("/parking/cam1/snapshot")

    assert r.status_code == 200

    data = r.json()

    assert data["num_occupied"] == 1
    assert data["num_places"] == 5
    assert len(data["places"]) == 1
    assert data["transform"]["valid"] is True


def test_snapshot_camera_not_found(client_unit):

    system = client_unit.app.state.parking_system

    def raise_error(_):
        raise RuntimeError("Camera not found")

    system.get_snapshot = raise_error

    r = client_unit.get("/parking/cam1/snapshot")

    assert r.status_code == 404


# =========================
# STATS
# =========================

def test_parking_stats_success(client_unit):

    system = client_unit.app.state.parking_system
    system.add_camera("cam1")

    r = client_unit.get("/parking/cam1/stats")

    assert r.status_code == 200

    data = r.json()

    assert data["occupied"] == 2
    assert data["free"] == 3
    assert data["total"] == 5


def test_parking_stats_not_found(client_unit):

    system = client_unit.app.state.parking_system

    def raise_error(_):
        raise RuntimeError("Camera not found")

    system.get_stats = raise_error

    r = client_unit.get("/parking/cam1/stats")

    assert r.status_code == 404


# =========================
# PROCESS FRAME
# =========================

def test_process_frame_success(client_unit):

    system = client_unit.app.state.parking_system
    system.add_camera("cam1")

    called = {"value": False}

    def fake_process_frame(cid, frame):
        called["value"] = True
        assert frame.shape[2] == 3

    system.process_frame = fake_process_frame

    r = client_unit.post(
        "/parking/cam1/frame",
        files={"file": ("frame.jpg", create_test_image(), "image/jpeg")}
    )

    assert r.status_code == 200
    assert called["value"] is True


def test_process_frame_invalid_image(client_unit):

    r = client_unit.post(
        "/parking/cam1/frame",
        files={"file": ("frame.jpg", create_invalid_image(), "image/jpeg")}
    )

    assert r.status_code == 400


def test_process_frame_camera_not_found(client_unit):

    system = client_unit.app.state.parking_system

    def raise_error(cid, frame):
        raise RuntimeError("Camera not found")

    system.process_frame = raise_error

    r = client_unit.post(
        "/parking/cam1/frame",
        files={"file": ("frame.jpg", create_test_image(), "image/jpeg")}
    )

    assert r.status_code == 404