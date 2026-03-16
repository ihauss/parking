class FakeParkingSystem:

    def __init__(self):
        self.cameras = {}

    def add_camera(self, cam_id):
        self.cameras[cam_id] = {
            "state": "RUNNING",
            "healthy": True,
            "latency": 10,
            "last_update": 0,
            "restart_count": 0
        }

    def list_cameras(self):
        return list(self.cameras.keys())

    def get_cam_state_str(self, cam_id):
        return self.cameras[cam_id]["state"]

    def is_healthy(self, cam_id):
        return self.cameras[cam_id]["healthy"]

    def get_stats(self, cam_id):
        return {"latency_ms": self.cameras[cam_id]["latency"]}

    def get_last_update_seconds(self, cam_id):
        return self.cameras[cam_id]["last_update"]

    def restart_camera(self, cam_id, config, ref):
        self.cameras[cam_id]["restart_count"] += 1