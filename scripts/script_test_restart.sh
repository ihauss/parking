#!/bin/bash

set -e

API_URL="http://127.0.0.1:8000"
DATA_DIR="data/cameras"
CAM_ID="restart_test_cam"

CONFIG_FILE="files/coords.json"
REFERENCE_FILE="files/reference.jpg"

echo "------------------------------------"
echo "Restart Test Starting"
echo "------------------------------------"

echo "Cleaning previous data..."
rm -rf $DATA_DIR/$CAM_ID || true

echo "Starting API..."
cd api
uvicorn app.main:app > restart_test.log 2>&1 &
API_PID=$!
cd ..

echo "Waiting for API to be ready..."

for i in {1..20}; do
    if curl -s "$API_URL/docs" > /dev/null; then
        echo "API is ready"
        break
    fi
    sleep 0.5
done

echo "Creating camera..."

curl -s -X POST "$API_URL/system/cameras" \
-F "camera_id=$CAM_ID" \
-F "config=@$CONFIG_FILE" \
-F "reference=@$REFERENCE_FILE" \
> /dev/null

sleep 1

echo "Checking camera exists..."

CAM_LIST=$(curl -s "$API_URL/system/cameras")

if [[ "$CAM_LIST" != *"$CAM_ID"* ]]; then
    echo "Camera creation failed"
    kill -9 $API_PID
    exit 1
fi

echo "Camera created successfully"

echo "Simulating crash..."
kill -9 $API_PID

sleep 2

echo "Restarting API..."
cd api
uvicorn app.main:app > restart_test.log 2>&1 &
API_PID=$!
cd ..

echo "Waiting for API to be ready..."

for i in {1..20}; do
    if curl -s "$API_URL/docs" > /dev/null; then
        echo "API is ready"
        break
    fi
    sleep 0.5
done

echo "Checking cameras after restart..."

CAM_LIST=$(curl -s "$API_URL/system/cameras")

if [[ "$CAM_LIST" != *"$CAM_ID"* ]]; then
    echo "Camera was NOT reloaded after restart"
    kill -9 $API_PID
    exit 1
fi

echo "Camera successfully reloaded after restart"

echo "Cleaning..."
curl -s -X DELETE "$API_URL/system/cameras/$CAM_ID"
kill -9 $API_PID

echo "------------------------------------"
echo "Restart Test PASSED"
echo "------------------------------------"