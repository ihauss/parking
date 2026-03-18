import threading
import time
import random
import logging
from app.FrameBuffer import FrameBuffer


class FrameDispatcher:
    """
    Dispatch frames from multiple cameras to the parking system.

    Design:
    - Each camera has a dedicated FrameBuffer storing only the latest frame
    - Older frames are dropped to prevent latency buildup (latest-frame strategy)
    - A single worker thread continuously processes available frames
    - Cameras are shuffled at each iteration to ensure fair processing

    This component is designed for real-time processing where latency is
    more critical than completeness.
    """

    def __init__(self, parking_system):
        """
        Initialize the FrameDispatcher.

        Args:
            parking_system: Backend system responsible for processing frames
        """
        self.parking_system = parking_system
        self.buffers = {}
        self.running = False

        self.logger = logging.getLogger(__name__)

        self.thread = threading.Thread(
            target=self._worker_loop,
            daemon=True
        )

    def start(self):
        """
        Start the worker thread.
        """
        self.running = True
        self.logger.info("FrameDispatcher started")
        self.thread.start()

    def stop(self):
        """
        Stop the worker thread gracefully.
        """
        self.running = False
        if self.thread.is_alive():
            self.thread.join()
        self.logger.info("FrameDispatcher stopped")

    def add_camera(self, camera_id):
        """
        Register a new camera.

        Args:
            camera_id (str): Unique identifier of the camera
        """
        self.buffers[camera_id] = FrameBuffer()
        self.logger.info(f"Camera added to dispatcher: {camera_id}")

    def remove_camera(self, camera_id):
        """
        Remove a camera from the dispatcher.

        Args:
            camera_id (str): Camera identifier
        """
        self.buffers.pop(camera_id, None)
        self.logger.info(f"Camera removed from dispatcher: {camera_id}")

    def push_frame(self, camera_id, frame):
        """
        Push a new frame into the buffer of a given camera.

        If a previous frame is still pending, it will be dropped.

        Args:
            camera_id (str): Camera identifier
            frame (np.ndarray): Frame image

        Raises:
            RuntimeError: If the camera is not registered
        """
        buffer = self.buffers.get(camera_id)

        if buffer is None:
            raise RuntimeError("Camera not registered")

        with buffer.lock:
            if buffer.latest_frame is not None:
                buffer.frames_dropped += 1

            buffer.latest_frame = frame
            buffer.frames_received += 1

    def _worker_loop(self):
        """
        Main worker loop.

        Continuously:
        - Iterates over all camera buffers
        - Processes the latest frame if available
        - Drops older frames implicitly
        """
        self.logger.info("FrameDispatcher worker loop started")

        while self.running:
            # Snapshot buffers to avoid concurrent modification issues
            items = list(self.buffers.items())

            # Shuffle to ensure fairness across cameras
            random.shuffle(items)

            for camera_id, buffer in items:

                # Retrieve and clear latest frame atomically
                with buffer.lock:
                    frame = buffer.latest_frame
                    buffer.latest_frame = None

                if frame is None:
                    continue

                try:
                    self.parking_system.process_frame(camera_id, frame)

                    # Update processed counter
                    with buffer.lock:
                        buffer.frames_processed += 1

                except Exception:
                    self.logger.exception(
                        f"Error processing frame for camera {camera_id}"
                    )

            # Small sleep to prevent CPU spinning
            time.sleep(0.001)

        self.logger.info("FrameDispatcher worker loop stopped")