from tests.integration.helpers import valid_image, corrupted_image
from tests.integration.helpers import valid_config, corrupted_json


def test_add_camera_invalid_json(client_integration):

    r = client_integration.delete("/system/cameras/cam1")

    r = client_integration.post(
        "/system/cameras",
        data={"camera_id": "cam1"},
        files={
            "config": ("config.json", corrupted_json(), "application/json"),
            "reference": ("ref.jpg", valid_image(), "image/jpeg"),
        },
    )

    assert r.status_code == 400
    assert r.json()["detail"] == "Invalid JSON configuration"


def test_add_camera_corrupted_reference(client_integration):

    r = client_integration.delete("/system/cameras/cam1")

    r = client_integration.post(
        "/system/cameras",
        data={"camera_id": "cam1"},
        files={
            "config": ("config.json", valid_config(), "application/json"),
            "reference": ("ref.jpg", corrupted_image(), "image/jpeg"),
        },
    )

    assert r.status_code == 500


def test_add_camera_invalid_id(client_integration):

    r = client_integration.delete("/system/cameras/cam1")

    r = client_integration.post(
        "/system/cameras",
        data={"camera_id": "cam#1"},
        files={
            "config": ("config.json", valid_config(), "application/json"),
            "reference": ("ref.jpg", valid_image(), "image/jpeg"),
        },
    )

    assert r.status_code == 400
    assert r.json()["detail"] == "Invalid camera_id format"


def test_process_frame_corrupted_image(client_integration):

    r = client_integration.delete("/system/cameras/cam1")

    client_integration.post(
        "/system/cameras",
        data={"camera_id": "cam1"},
        files={
            "config": ("config.json", valid_config(), "application/json"),
            "reference": ("ref.jpg", valid_image(), "image/jpeg"),
        },
    )

    r = client_integration.post(
        "/parking/cam1/frame",
        files={
            "file": ("frame.jpg", corrupted_image(), "image/jpeg")
        },
    )

    assert r.status_code == 400

    r = client_integration.delete("/system/cameras/cam1")