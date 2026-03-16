import asyncio
import pytest
from time import sleep

from tests.integration.helpers import valid_image, valid_config


CAMERAS = ["hf_cam1", "hf_cam2", "hf_cam3"]
FRAMES_PER_CAM = 100


async def send_frame(client, cam):
    r = client.post(
        f"/parking/{cam}/frame",
        files={"file": ("frame.jpg", valid_image(), "image/jpeg")},
    )
    assert r.status_code == 200


async def wait_until_processed(dispatcher, cameras, timeout=5):

    start = asyncio.get_event_loop().time()

    while True:

        done = True

        for cam in cameras:
            buffer = dispatcher.buffers[cam]

            with buffer.lock:
                if buffer.frames_processed < buffer.frames_received:
                    done = False
                    break

        if done:
            return

        if asyncio.get_event_loop().time() - start > timeout:
            raise TimeoutError("Frames not processed in time")

        await asyncio.sleep(0.01)


@pytest.mark.anyio
async def test_high_frequency_multicam(client_integration):

    # ======================
    # create cameras
    # ======================

    for cam in CAMERAS:

        client_integration.delete(f"/system/cameras/{cam}")

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
    # send frames concurrently
    # ======================

    tasks = []

    for _ in range(FRAMES_PER_CAM):
        for cam in CAMERAS:
            tasks.append(send_frame(client_integration, cam))

    await asyncio.gather(*tasks)

    # ======================
    # verify cameras still alive
    # ======================

    for cam in CAMERAS:

        stats = client_integration.get(f"/parking/{cam}/stats")
        snapshot = client_integration.get(f"/parking/{cam}/snapshot")

        assert stats.status_code == 200
        assert snapshot.status_code == 200

    # ======================
    # cleanup
    # ======================

    dispatcher = client_integration.app.state.frame_dispatcher
    await wait_until_processed(dispatcher, CAMERAS)

    for cam in CAMERAS:
        client_integration.delete(f"/system/cameras/{cam}")