from tests.integration.helpers import valid_image, valid_config

def test_stress_multicam(client_integration):

    cameras = ["cam1", "cam2", "cam3"]

    # ======================
    # create cameras
    # ======================

    for cam in cameras:
        r = client_integration.delete(f"/system/cameras/{cam}")
        r = client_integration.post(
            "/system/cameras",
            data={"camera_id": cam},
            files={
                "config": ("config.json", valid_config(), "application/json"),
                "reference": ("ref.jpg", valid_image(), "image/jpeg"),
            },
        )

        assert r.status_code == 201

    # ======================
    # send frames
    # ======================

    for _ in range(50):

        for cam in cameras:

            r = client_integration.post(
                f"/parking/{cam}/frame",
                files={"file": ("frame.jpg", valid_image(), "image/jpeg")},
            )

            stats = client_integration.get(f"/parking/{cam}/stats")
            snapshot = client_integration.get(f"/parking/{cam}/snapshot")

            assert r.status_code == 200
            assert stats.status_code == 200
            assert snapshot.status_code == 200

    # ======================
    # verify all cameras alive
    # ======================

    for cam in cameras:

        stats = client_integration.get(f"/parking/{cam}/stats")
        snapshot = client_integration.get(f"/parking/{cam}/snapshot")

        assert stats.status_code == 200
        assert snapshot.status_code == 200
        r = client_integration.delete(f"/system/cameras/{cam}")