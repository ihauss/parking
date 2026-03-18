import threading


class FrameBuffer:
    """
    Thread-safe buffer storing the latest frame for a single camera.

    Design:
    - Only the most recent frame is kept (latest-frame strategy)
    - Older frames are dropped to avoid processing backlog and latency
    - Counters are maintained for observability and debugging

    Attributes:
        lock (threading.Lock): Protects access to shared data
        latest_frame (np.ndarray | None): Most recent frame received
        frames_received (int): Total number of frames received
        frames_processed (int): Total number of frames processed
        frames_dropped (int): Number of frames overwritten before processing
    """

    def __init__(self):
        # Lock to ensure thread-safe access to the buffer
        self.lock = threading.Lock()

        # Stores the most recent frame (None if no frame available)
        self.latest_frame = None

        # Metrics for monitoring and debugging
        self.frames_received = 0
        self.frames_processed = 0
        self.frames_dropped = 0