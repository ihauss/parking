import io
import shutil
import pytest
from pathlib import Path

from app.main import app
from tests.unit.helpers import create_test_image, create_test_config


# -----------------------------
# Fixtures
# -----------------------------

@pytest.fixture(autouse=True)
def clean_data_dir():
    data_dir = Path("data")
    if data_dir.exists():
        shutil.rmtree(data_dir)
    yield
    if data_dir.exists():
        shutil.rmtree(data_dir)


# -----------------------------
# Tests
# -----------------------------

def test_add_camera_success(client_unit):

    response = client_unit.post(
        "/system/cameras",
        data={"camera_id": "cam1"},
        files={
            "config": ("config.json", create_test_config(), "application/json"),
            "reference": ("ref.jpg", create_test_image(), "image/jpeg"),
        },
    )

    assert response.status_code == 201
    assert response.json()["detail"] == "Camera created"


def test_add_camera_duplicate(client_unit):

    client_unit.post(
        "/system/cameras",
        data={"camera_id": "cam1"},
        files={
            "config": ("config.json", create_test_config(), "application/json"),
            "reference": ("ref.jpg", create_test_image(), "image/jpeg"),
        },
    )

    response = client_unit.post(
        "/system/cameras",
        data={"camera_id": "cam1"},
        files={
            "config": ("config.json", create_test_config(), "application/json"),
            "reference": ("ref.jpg", create_test_image(), "image/jpeg"),
        },
    )

    assert response.status_code == 409


def test_add_camera_invalid_json(client_unit):

    bad_json = io.BytesIO(b"{invalid}")

    response = client_unit.post(
        "/system/cameras",
        data={"camera_id": "cam1"},
        files={
            "config": ("config.json", bad_json, "application/json"),
            "reference": ("ref.jpg", create_test_image(), "image/jpeg"),
        },
    )

    assert response.status_code == 400


def test_list_cameras(client_unit):

    client_unit.post(
        "/system/cameras",
        data={"camera_id": "cam1"},
        files={
            "config": ("config.json", create_test_config(), "application/json"),
            "reference": ("ref.jpg", create_test_image(), "image/jpeg"),
        },
    )

    response = client_unit.get("/system/cameras")

    assert response.status_code == 200
    data = response.json()

    assert len(data) == 1
    assert data[0]["id"] == "cam1"
    assert data[0]["healthy"] is True


def test_camera_health(client_unit):

    client_unit.post(
        "/system/cameras",
        data={"camera_id": "cam1"},
        files={
            "config": ("config.json", create_test_config(), "application/json"),
            "reference": ("ref.jpg", create_test_image(), "image/jpeg"),
        },
    )

    response = client_unit.get("/system/cameras/cam1/health")

    assert response.status_code == 200
    assert response.json()["healthy"] is True


def test_camera_state(client_unit):

    client_unit.post(
        "/system/cameras",
        data={"camera_id": "cam1"},
        files={
            "config": ("config.json", create_test_config(), "application/json"),
            "reference": ("ref.jpg", create_test_image(), "image/jpeg"),
        },
    )

    response = client_unit.get("/system/cameras/cam1/state")

    assert response.status_code == 200

    data = response.json()

    assert data["state"] == "RUNNING"
    assert data["healthy"] is True


def test_remove_camera(client_unit):

    client_unit.post(
        "/system/cameras",
        data={"camera_id": "cam1"},
        files={
            "config": ("config.json", create_test_config(), "application/json"),
            "reference": ("ref.jpg", create_test_image(), "image/jpeg"),
        },
    )

    response = client_unit.delete("/system/cameras/cam1")

    assert response.status_code == 204

    response = client_unit.get("/system/cameras")

    assert response.json() == []


def test_restart_camera(client_unit):

    client_unit.post(
        "/system/cameras",
        data={"camera_id": "cam1"},
        files={
            "config": ("config.json", create_test_config(), "application/json"),
            "reference": ("ref.jpg", create_test_image(), "image/jpeg"),
        },
    )

    response = client_unit.post("/system/cameras/cam1/restart")

    assert response.status_code == 200
    assert response.json()["detail"] == "Camera restarted"