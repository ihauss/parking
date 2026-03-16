import threading
import time
from app.FrameBuffer import FrameBuffer


class FrameDispatcher:

    def __init__(self, parking_system):

        self.parking_system = parking_system
        self.buffers = {}

        self.thread = threading.Thread(
            target=self._worker_loop,
            daemon=True
        )

    def start(self):
        self.thread.start()

    def add_camera(self, camera_id):
        self.buffers[camera_id] = FrameBuffer()

    def remove_camera(self, camera_id):
        self.buffers.pop(camera_id, None)

    def push_frame(self, camera_id, frame):
        buffer = self.buffers.get(camera_id)

        if buffer is None:
            raise RuntimeError("Camera not registered")

        with buffer.lock:
            if buffer.latest_frame is not None:
                buffer.frames_dropped += 1
            buffer.latest_frame = frame
            buffer.frames_received += 1

    def _worker_loop(self):
        while True:
            for camera_id, buffer in self.buffers.items():

                with buffer.lock:
                    frame = buffer.latest_frame
                    buffer.latest_frame = None

                if frame is None:
                    continue

                self.parking_system.process_frame(camera_id, frame)

                buffer.frames_processed += 1

            time.sleep(0.001)