import argparse
import time
import requests
import cv2
import json
import os
from datetime import datetime

API_URL = "http://localhost:8000"

VIDEO_REFERENCE = "files/video1.mp4"
REFERENCE_IMAGE = "files/reference.jpg"
CONFIG_FILE = "files/coords.json"


# ---------- Utils ----------

def log(msg):
    now = datetime.now().strftime("%H:%M:%S")
    print(f"[{now}] {msg}")


def safe_request(method, url, **kwargs):
    try:
        r = requests.request(method, url, timeout=2, **kwargs)
        return r
    except Exception as e:
        log(f"Request error: {e}")
        return None


# ---------- API Calls ----------

def create_camera(camera_id):
    url = f"{API_URL}/system/cameras"

    log(f"Creating camera [{camera_id}]")

    files = {
        "reference": (
            "ref.jpg",
            open(REFERENCE_IMAGE, "rb"),
            "image/jpeg"
        ),
        "config": (
            "config.json",
            open(CONFIG_FILE, "rb"),
            "application/json"
        )
    }

    data = {
        "camera_id": camera_id
    }

    r = safe_request("POST", url, data=data, files=files)

    if r:
        log(f"→ Status: {r.status_code}")
        if r.status_code != 201:
            log(f"→ Error: {r.text}")

    return r


def delete_camera(camera_id):
    url = f"{API_URL}/system/cameras/{camera_id}"

    log(f"Deleting camera [{camera_id}]")
    r = safe_request("DELETE", url)

    if r:
        log(f"→ Status: {r.status_code}")
    return r


def send_frame(camera_id, frame_bytes):
    url = f"{API_URL}/parking/{camera_id}/frame"

    files = {
        "file": ("frame.jpg", frame_bytes, "image/jpeg")
    }

    return safe_request("POST", url, files=files)


def get_metrics(camera_id):
    url = f"{API_URL}/metrics/{camera_id}"
    r = safe_request("GET", url)

    if r and r.status_code == 200:
        return r.json()
    return None


def get_stats(camera_id):
    url = f"{API_URL}/parking/{camera_id}/stats"
    r = safe_request("GET", url)

    if r and r.status_code == 200:
        return r.json()
    return None


# ---------- Main Cycle ----------

def run_cycle(camera_id):
    if not os.path.exists(REFERENCE_IMAGE) or not os.path.exists(CONFIG_FILE):
        log("Missing reference.jpg or config.json")
        return

    cap = cv2.VideoCapture(VIDEO_REFERENCE)

    if not cap.isOpened():
        log("Cannot open reference image/video")
        return

    create_camera(camera_id)
    time.sleep(1)

    log("Starting frame cycle (20 frames / 10 seconds)\n")

    start_time = time.time()
    frame_count = 0

    while frame_count < 20:
        ret, frame = cap.read()

        if not ret:
            cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
            continue

        _, buffer = cv2.imencode(".jpg", frame)

        r = send_frame(camera_id, buffer.tobytes())
        frame_count += 1

        # Fetch metrics + stats
        metrics = get_metrics(camera_id)
        stats = get_stats(camera_id)

        # Extract useful info
        fps = metrics.get("fps") if metrics else None
        latency = metrics.get("latency_ms") if metrics else None
        occupied = stats.get("occupied") if stats else None
        total = stats.get("total") if stats else None

        log(
            f"Frame {frame_count:02d} | "
            f"status={r.status_code if r else 'ERR'} | "
            f"FPS={fps} | Latency={latency} ms | "
            f"Occupied={occupied}/{total}"
        )

        # Spread 20 frames over ~10 seconds
        time.sleep(0.5)

    duration = time.time() - start_time
    log(f"\nCycle complete in {duration:.2f}s")

    delete_camera(camera_id)
    cap.release()


# ---------- Entry ----------

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--camera_id", required=True)

    args = parser.parse_args()
    run_cycle(args.camera_id)


if __name__ == "__main__":
    main()