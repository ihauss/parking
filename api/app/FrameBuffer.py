import threading


class FrameBuffer:
    def __init__(self):
        self.lock = threading.Lock()
        self.latest_frame = None
        self.frames_received = 0
        self.frames_processed = 0
        self.frames_dropped = 0