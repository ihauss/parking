#include "smart_parking/ParkingSystem.h"

#include <stdexcept>

//
// --- Camera Management ---
//

void ParkingSystem::addCamera(
    const std::string& id,
    const std::string& jsonPath,
    const cv::Mat& reference
){
    std::lock_guard<std::mutex> lock(_mutex);

    if (_cameras.count(id)) {
        throw std::runtime_error("Camera already exists: " + id);
    }

    auto ctx = std::make_shared<CameraContext>();
    ctx->engine = std::make_unique<Parking>(jsonPath, reference);
    ctx->lastUpdate = std::chrono::steady_clock::now();
    ctx->healthy = true;
    ctx->errorCount = 0;

    _cameras.emplace(id, ctx);
}

void ParkingSystem::removeCamera(const std::string& id){
    std::lock_guard<std::mutex> lock(_mutex);

    auto it = _cameras.find(id);
    if (it == _cameras.end()) {
        throw std::runtime_error("Camera not found: " + id);
    }

    _cameras.erase(it);
}

void ParkingSystem::restartCamera(
    const std::string& id,
    const std::string& jsonPath,
    const cv::Mat& reference
){
    std::shared_ptr<CameraContext> ctx;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cameras.find(id);
        if (it == _cameras.end())
            throw std::runtime_error("Camera not found");
        ctx = it->second;
    }

    std::lock_guard<std::mutex> camLock(ctx->mutex);

    ctx->engine = std::make_unique<Parking>(jsonPath, reference);
    ctx->healthy = true;
    ctx->errorCount = 0;
    ctx->lastUpdate = std::chrono::steady_clock::now();
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

    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cameras.find(id);
        if (it == _cameras.end())
            throw std::runtime_error("Camera not found");
        ctx = it->second;
    }

    std::lock_guard<std::mutex> camLock(ctx->mutex);

    if (!ctx->healthy)
        return;

    try {
        ctx->engine->evolve(frame);
        ctx->lastUpdate = std::chrono::steady_clock::now();
        ctx->errorCount = 0;
    }
    catch (const std::exception& e){
        ctx->errorCount++;

        if (ctx->errorCount > 5)
            ctx->healthy = false;

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
        if (it == _cameras.end())
            throw std::runtime_error("Camera not found");
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
        if (it == _cameras.end())
            throw std::runtime_error("Camera not found");
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
        if (it == _cameras.end())
            throw std::runtime_error("Camera not found");
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
        if (it == _cameras.end())
            throw std::runtime_error("Camera not found");
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
        if (it == _cameras.end())
            throw std::runtime_error("Camera not found");
        ctx = it->second;
    }

    std::lock_guard<std::mutex> camLock(ctx->mutex);
    return ctx->engine->getStateString();
}

std::chrono::steady_clock::time_point
ParkingSystem::getLastUpdate(const std::string& id) const {
    std::shared_ptr<CameraContext> ctx;

    {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _cameras.find(id);
        if (it == _cameras.end())
            throw std::runtime_error("Camera not found");
        ctx = it->second;
    }

    std::lock_guard<std::mutex> camLock(ctx->mutex);
    return ctx->lastUpdate;
}

double ParkingSystem::getLastUpdateSeconds(const std::string& id) const {
    auto ctx = getCameraContext(id);
    auto now = std::chrono::steady_clock::now();

    return std::chrono::duration<double>(now - ctx->lastUpdate).count();
}