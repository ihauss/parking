#include "smart_parking/ParkingSystem.h"

#include <stdexcept>

namespace smart_parking {

//
// --- Camera Management ---
//

void ParkingSystem::addCamera(
    const std::string& id,
    const std::string& jsonPath,
    const cv::Mat& reference
){
    // Global map protection
    std::lock_guard<std::mutex> lock(_mutex);

    // Prevent duplicate camera IDs
    if (_cameras.count(id)) {
        Logger::log().error("Camera already exists: " + id);
        throw std::runtime_error("Camera already exists: " + id);
    }

    // Create new camera context
    auto ctx = std::make_shared<CameraContext>();

    // Initialize parking engine
    ctx->engine = std::make_unique<Parking>(jsonPath, reference);

    // Initialize monitoring state
    ctx->lastUpdate = std::chrono::steady_clock::now();
    ctx->healthy = true;
    ctx->errorCount = 0;

    // Register camera
    _cameras.emplace(id, ctx);

    Logger::log().info("Camera added: " + id);
}

void ParkingSystem::removeCamera(const std::string& id){
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _cameras.find(id);
    if (it == _cameras.end()) {
        Logger::log().error("Camera not found: " + id);
        throw std::runtime_error("Camera not found: " + id);
    }

    _cameras.erase(it);

    Logger::log().info("Camera removed: " + id);
}

void ParkingSystem::restartCamera(
    const std::string& id,
    const std::string& jsonPath,
    const cv::Mat& reference
){
    std::shared_ptr<CameraContext> ctx;

    // Access shared map
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cameras.find(id);
        if (it == _cameras.end()) {
            Logger::log().error("Camera not found: " + id);
            throw std::runtime_error("Camera not found");
        }
        ctx = it->second;
    }

    // Lock specific camera
    std::lock_guard<std::mutex> camLock(ctx->mutex);

    // Recreate engine (full reset)
    ctx->engine = std::make_unique<Parking>(jsonPath, reference);

    // Reset monitoring state
    ctx->healthy = true;
    ctx->errorCount = 0;
    ctx->lastUpdate = std::chrono::steady_clock::now();

    Logger::log().warn("Camera restarted: " + id);
}

std::vector<std::string> ParkingSystem::listCameras() const {
    std::lock_guard<std::mutex> lock(_mutex);

    std::vector<std::string> ids;
    ids.reserve(_cameras.size());

    for (const auto& pair : _cameras) {
        ids.push_back(pair.first);
    }

    return ids;
}

//
// --- Processing ---
//

void ParkingSystem::processFrame(
    const std::string& id,
    const Frame& frame
){
    std::shared_ptr<CameraContext> ctx;

    // Retrieve camera context safely
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cameras.find(id);
        if (it == _cameras.end()) {
            Logger::log().error("Camera not found: " + id);
            throw std::runtime_error("Camera not found");
        }
        ctx = it->second;
    }

    // Lock camera-specific resources
    std::lock_guard<std::mutex> camLock(ctx->mutex);

    // Skip processing if camera is unhealthy
    if (!ctx->healthy) {
        Logger::log().warn("Skipping frame: camera unhealthy (" + id + ")");
        return;
    }

    try {
        // Main pipeline execution
        ctx->engine->evolve(frame);

        // Update monitoring data
        ctx->lastUpdate = std::chrono::steady_clock::now();
        ctx->errorCount = 0;
    }
    catch (const std::exception& e){
        ctx->errorCount++;

        Logger::log().error(
            "Camera " + id +
            " processing error (" + std::to_string(ctx->errorCount) + "): " +
            e.what()
        );

        // Mark camera unhealthy after repeated failures
        if (ctx->errorCount > 5) {
            ctx->healthy = false;
            Logger::log().error("Camera marked unhealthy: " + id);
        }

        throw; 
    }
}

//
// --- Retrieval ---
//

RenderSnapshot ParkingSystem::getSnapshot(const std::string& id) const {
    std::shared_ptr<CameraContext> ctx;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cameras.find(id);
        if (it == _cameras.end()) {
            Logger::log().error("Camera not found: " + id);
            throw std::runtime_error("Camera not found");
        }
        ctx = it->second;
    }

    std::lock_guard<std::mutex> camLock(ctx->mutex);
    return ctx->engine->getRenderSnapshot();
}

std::map<std::string, double>
ParkingSystem::getStats(const std::string& id) const {
    std::shared_ptr<CameraContext> ctx;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cameras.find(id);
        if (it == _cameras.end()) {
            Logger::log().error("Camera not found: " + id);
            throw std::runtime_error("Camera not found");
        }
        ctx = it->second;
    }

    std::lock_guard<std::mutex> camLock(ctx->mutex);
    return ctx->engine->getStats();
}

//
// --- Monitoring ---
//

bool ParkingSystem::isHealthy(const std::string& id) const {
    std::shared_ptr<CameraContext> ctx;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cameras.find(id);
        if (it == _cameras.end()) {
            Logger::log().error("Camera not found: " + id);
            throw std::runtime_error("Camera not found");
        }
        ctx = it->second;
    }

    std::lock_guard<std::mutex> camLock(ctx->mutex);
    return ctx->healthy;
}

CameraState ParkingSystem::getState(const std::string& id) const{
    std::shared_ptr<CameraContext> ctx;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cameras.find(id);
        if (it == _cameras.end()) {
            Logger::log().error("Camera not found: " + id);
            throw std::runtime_error("Camera not found");
        }
        ctx = it->second;
    }

    std::lock_guard<std::mutex> camLock(ctx->mutex);
    return ctx->engine->getState();
}

std::string ParkingSystem::getStateString(const std::string& id) const{
    std::shared_ptr<CameraContext> ctx;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cameras.find(id);
        if (it == _cameras.end()) {
            Logger::log().error("Camera not found: " + id);
            throw std::runtime_error("Camera not found");
        }
        ctx = it->second;
    }

    std::lock_guard<std::mutex> camLock(ctx->mutex);
    return to_string(ctx->engine->getState());
}

std::chrono::steady_clock::time_point
ParkingSystem::getLastUpdate(const std::string& id) const {
    std::shared_ptr<CameraContext> ctx;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cameras.find(id);
        if (it == _cameras.end()) {
            Logger::log().error("Camera not found: " + id);
            throw std::runtime_error("Camera not found");
        }
        ctx = it->second;
    }

    std::lock_guard<std::mutex> camLock(ctx->mutex);
    return ctx->lastUpdate;
}

double ParkingSystem::getLastUpdateSeconds(const std::string& id) const {
    std::shared_ptr<CameraContext> ctx;

    // Access global map
    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cameras.find(id);
        if (it == _cameras.end()) {
            Logger::log().error("Camera not found: " + id);
            throw std::runtime_error("Camera not found");
        }
        ctx = it->second;
    }

    // Access camera context
    std::chrono::steady_clock::time_point lastUpdate;
    {
        std::lock_guard<std::mutex> camLock(ctx->mutex);
        lastUpdate = ctx->lastUpdate;
    }

    // Compute elapsed time since last update
    auto now = std::chrono::steady_clock::now();

    return std::chrono::duration<double>(now - lastUpdate).count();
}

} // namespace smart_parking