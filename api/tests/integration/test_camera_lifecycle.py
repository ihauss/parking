from tests.integration.helpers import create_dummy_image, create_dummy_config


def test_camera_lifecycle(client_integration):

    camera_id = "cam_test"

    r = client_integration.delete(f"/system/cameras/{camera_id}")

    r = client_integration.get("/system/cameras")

    # CREATE
    r = client_integration.post(
        "/system/cameras",
        data={"camera_id": camera_id},
        files={
            "reference": ("ref.jpg", create_dummy_image(), "image/jpeg"),
            "config": ("conf.json", create_dummy_config(), "application/json"),
        },
    )

    assert r.status_code == 201


    # CHECK CAMERA EXISTS
    r = client_integration.get("/system/cameras")

    assert any(c["id"] == camera_id for c in r.json())

    r = client_integration.get(f"/system/cameras/{camera_id}/state")

    assert r.status_code == 200
    assert r.json()["state"] == "IDLE"

    for i in range(5):
        r = client_integration.post(
            f"/parking/{camera_id}/frame",
            files={"file": ("frame.jpg", create_dummy_image(), "image/jpeg")}
        )

    assert r.status_code == 200

    r = client_integration.get(f"/system/cameras/{camera_id}/state")

    assert r.status_code == 200
    assert r.json()["state"] == "RUNNING"

    # RESTART
    r = client_integration.post(f"/system/cameras/{camera_id}/restart")

    assert r.status_code == 200

    r = client_integration.get(f"/system/cameras/{camera_id}/state")

    assert r.status_code == 200
    assert r.json()["state"] == "IDLE"

    # DELETE
    r = client_integration.delete(f"/system/cameras/{camera_id}")

    assert r.status_code == 204