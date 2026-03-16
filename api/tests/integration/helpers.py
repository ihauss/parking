import io
import json
from PIL import Image
import numpy as np
import cv2


def create_dummy_image():
    img = Image.new("RGB", (32, 32), color=(255, 255, 255))
    buf = io.BytesIO()
    img.save(buf, format="JPEG")
    buf.seek(0)
    return buf


def create_dummy_config():
    config = {
        "parking_places": [
            {
                "coords": [
                    {"x": 0, "y": 0},
                    {"x": 10, "y": 0},
                    {"x": 10, "y": 10},
                    {"x": 0, "y": 10},
                ]
            }
        ]
    }

    return io.BytesIO(json.dumps(config).encode())

def valid_image():
    img = np.zeros((50, 50, 3), dtype=np.uint8)
    _, buf = cv2.imencode(".jpg", img)
    return io.BytesIO(buf.tobytes())


def corrupted_image():
    return io.BytesIO(b"this_is_not_an_image")


def valid_config():
    return io.BytesIO(json.dumps({"parking_places": []}).encode())


def corrupted_json():
    return io.BytesIO(b"{invalid_json}")