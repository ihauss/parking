import numpy as np
import cv2
import io
import json

def create_test_image():
    img = np.zeros((50, 50, 3), dtype=np.uint8)
    _, buf = cv2.imencode(".jpg", img)
    return io.BytesIO(buf.tobytes())


def create_test_config():
    data = {"places": []}
    return io.BytesIO(json.dumps(data).encode())

def create_invalid_image():
    return io.BytesIO(b"notanimage")