from tests.integration.helpers import valid_image, valid_config

def test_stress_single_camera(client_integration):

    r = client_integration.delete("/system/cameras/cam1")

    r = client_integration.post(
        "/system/cameras",
        data={"camera_id": "cam1"},
        files={
            "config": ("config.json", valid_config(), "application/json"),
            "reference": ("ref.jpg", valid_image(), "image/jpeg"),
        },
    )

    assert r.status_code == 201

    # ======================
    # send many frames
    # ======================

    for _ in range(200):

        r = client_integration.post(
            "/parking/cam1/frame",
            files={"file": ("frame.jpg", valid_image(), "image/jpeg")},
        )

        assert r.status_code == 200

    # ======================
    # system still works
    # ======================

    stats = client_integration.get("/parking/cam1/stats")
    snapshot = client_integration.get("/parking/cam1/snapshot")

    assert stats.status_code == 200
    assert snapshot.status_code == 200

    r = client_integration.delete("/system/cameras/cam1")