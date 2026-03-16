from tests.integration.helpers import valid_image, valid_config

def test_multicam_isolation(client_integration):

    # =========================
    # ADD TWO CAMERAS
    # =========================

    r = client_integration.delete("/system/cameras/cam1")
    r = client_integration.delete("/system/cameras/cam2")

    r1 = client_integration.post(
        "/system/cameras",
        data={"camera_id": "cam1"},
        files={
            "config": ("config.json", valid_config(), "application/json"),
            "reference": ("ref.jpg", valid_image(), "image/jpeg"),
        },
    )

    r2 = client_integration.post(
        "/system/cameras",
        data={"camera_id": "cam2"},
        files={
            "config": ("config.json", valid_config(), "application/json"),
            "reference": ("ref.jpg", valid_image(), "image/jpeg"),
        },
    )

    assert r1.status_code == 201
    assert r2.status_code == 201

    # =========================
    # INITIAL STATS
    # =========================
    stats1_before = client_integration.get(f"/system/cameras/cam1/state").json()["state"]
    stats2_before = client_integration.get(f"/system/cameras/cam2/state").json()["state"]

    # =========================
    # SEND FRAME TO CAM1
    # =========================

    r = client_integration.post(
        "/parking/cam1/frame",
        files={
            "file": ("frame.jpg", valid_image(), "image/jpeg")
        },
    )

    assert r.status_code == 200

    # =========================
    # STATS AFTER
    # =========================

    stats1_after = client_integration.get(f"/system/cameras/cam1/state").json()["state"]
    stats2_after = client_integration.get(f"/system/cameras/cam2/state").json()["state"]

    # cam1 peut changer
    assert stats1_after is not None

    # cam2 doit rester IDENTIQUE
    assert stats1_before != stats1_after
    assert stats2_before == stats2_after

    r = client_integration.delete("/system/cameras/cam1")
    r = client_integration.delete("/system/cameras/cam2")