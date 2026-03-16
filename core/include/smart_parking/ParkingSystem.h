#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <mutex>
#include <vector>
#include <chrono>
#include <map>

#include "smart_parking/Parking.h"
#include "smart_parking/Frame.h"
#include "smart_parking/RenderSnapshot.h"
#include "smart_parking/CameraState.h"

struct CameraContext {
    std::unique_ptr<Parking> engine;
    std::mutex mutex;   // nouveau
    std::chrono::steady_clock::time_point lastUpdate;
    bool healthy;
    int errorCount;
};

class ParkingSystem {
public:
    ParkingSystem() = default;

    // --- Camera management ---
    void addCamera(
        const std::string& id,
        const std::string& jsonPath,
        const cv::Mat& reference
    );

    void removeCamera(const std::string& id);

    void restartCamera(
        const std::string& id,
        const std::string& jsonPath,
        const cv::Mat& reference
    );

    std::vector<std::string> listCameras() const;

    // --- Processing ---
    void processFrame(
        const std::string& id,
        const Frame& frame
    );

    // --- Retrieval ---
    RenderSnapshot getSnapshot(const std::string& id) const;

    std::map<std::string, double> getStats(const std::string& id) const;

    // --- Monitoring ---
    bool isHealthy(const std::string& id) const;

    CameraState getState(const std::string& id) const;
    std::string getStateString(const std::string& id) const;

    std::chrono::steady_clock::time_point
    getLastUpdate(const std::string& id) const;
    double getLastUpdateSeconds(const std::string& id) const;

private:
    std::unordered_map<std::string, std::shared_ptr<CameraContext>> _cameras;
    mutable std::mutex _mutex;
};