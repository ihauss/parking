import asyncio
import cv2
from pathlib import Path
import time

DATA_DIR = Path("data")
CAMERAS_DIR = DATA_DIR / "cameras"


class CameraWatchdog:

    def __init__(self, parking_system, logger, timeout=5):
        self.parking_system = parking_system
        self.logger = logger
        self.timeout = timeout
        self.running = True

        self.last_restart = {}
        self.references = {}

    def _iteration(self):
        try:
            cameras = self.parking_system.list_cameras()

            for cam_id in list(self.references):
                if cam_id not in cameras:
                    del self.references[cam_id]

            for cam_id in cameras:

                need_restart = False

                camera_dir = CAMERAS_DIR / cam_id
                config_path = camera_dir / "config.json"
                reference_path = camera_dir / "reference.jpg"

                if not config_path.exists() or not reference_path.exists():
                    self.logger.warning(
                        "Missing camera files for %s", cam_id
                    )
                    continue

                state = self.parking_system.get_cam_state_str(cam_id)
                healthy = self.parking_system.is_healthy(cam_id)

                stats = self.parking_system.get_stats(cam_id)
                latency_ms = stats["latency_ms"]

                last_frame_age = (
                    self.parking_system.get_last_update_seconds(cam_id)
                )

                # Load reference once
                if cam_id not in self.references:

                    ref = cv2.imread(str(reference_path))

                    if ref is None:
                        self.logger.warning(
                            "Invalid reference image %s", reference_path
                        )
                        continue

                    self.references[cam_id] = ref

                ref = self.references[cam_id]

                if not healthy:
                    need_restart = True

                elif state == "ERROR" and last_frame_age > 2:
                    need_restart = True

                elif last_frame_age > self.timeout and state == "RUNNING":
                    need_restart = True

                elif latency_ms > 500 and last_frame_age > 1:
                    need_restart = True

                if need_restart:

                    now = time.time()

                    if now - self.last_restart.get(cam_id, 0) < 10:
                        continue

                    self.logger.warning(
                        "Restarting camera %s | state=%s age=%.2fs latency=%.1fms",
                        cam_id,
                        state,
                        last_frame_age,
                        latency_ms
                    )

                    self.parking_system.restart_camera(
                        cam_id,
                        str(config_path),
                        ref
                    )

                    self.last_restart[cam_id] = now

        except Exception:
            self.logger.exception("Watchdog failure")

    async def run(self):

        CAMERAS_DIR.mkdir(parents=True, exist_ok=True)

        while self.running:
            
            self._iteration()

            await asyncio.sleep(1)