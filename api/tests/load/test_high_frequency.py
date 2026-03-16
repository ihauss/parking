import asyncio
import pytest
import time

from tests.integration.helpers import valid_image, valid_config


CAMERA_COUNT = 20
FPS = 5
DURATION = 5  # secondes
CAMERAS = [f"hf_cam{i}" for i in range(CAMERA_COUNT)]


async def send_frame(client, cam):
    r = client.post(
        f"/parking/{cam}/frame",
        files={"file": ("frame.jpg", valid_image(), "image/jpeg")},
    )
    assert r.status_code == 200


async def camera_loop(client, cam):

    interval = 1.0 / FPS
    end_time = time.time() + DURATION

    while time.time() < end_time:

        await send_frame(client, cam)

        await asyncio.sleep(interval)


async def wait_until_processed(dispatcher, cameras, timeout=10):

    start = time.time()

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

        if time.time() - start > timeout:
            raise TimeoutError("Processing timeout")

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
    # run camera streams
    # ======================

    tasks = []

    for cam in CAMERAS:
        tasks.append(camera_loop(client_integration, cam))

    await asyncio.gather(*tasks)

    # ======================
    # wait processing
    # ======================

    dispatcher = client_integration.app.state.frame_dispatcher

    await wait_until_processed(dispatcher, CAMERAS)

    # ======================
    # verify cameras alive
    # ======================

    for cam in CAMERAS:

        stats = client_integration.get(f"/parking/{cam}/stats")
        snapshot = client_integration.get(f"/parking/{cam}/snapshot")

        assert stats.status_code == 200
        assert snapshot.status_code == 200

    # ======================
    # cleanup
    # ======================

    for cam in CAMERAS:
        client_integration.delete(f"/system/cameras/{cam}")