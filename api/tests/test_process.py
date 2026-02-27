import io
import numpy as np
import cv2


def test_process_ok(client):
    img = np.zeros((100, 100, 3), dtype=np.uint8)
    _, buf = cv2.imencode(".jpg", img)

    response = client.post(
        "/parking/process",
        files={"file": ("test.jpg", buf.tobytes(), "image/jpeg")},
    )

    assert response.status_code == 200
    assert response.json()["status"] == "ok"


def test_process_empty_file(client):
    response = client.post(
        "/parking/process",
        files={"file": ("empty.jpg", b"", "image/jpeg")},
    )

    assert response.status_code == 400
    assert response.json()["detail"] == "Empty image file"


def test_process_invalid_image(client):
    response = client.post(
        "/parking/process",
        files={"file": ("bad.txt", b"not_an_image", "text/plain")},
    )

    assert response.status_code == 400
    assert response.json()["detail"] == "Invalid image"


def test_process_not_initialized(client_no_parking):
    img = np.zeros((10, 10, 3), dtype=np.uint8)
    _, buf = cv2.imencode(".jpg", img)

    response = client_no_parking.post(
        "/parking/process",
        files={"file": ("test.jpg", buf.tobytes(), "image/jpeg")},
    )

    assert response.status_code == 503