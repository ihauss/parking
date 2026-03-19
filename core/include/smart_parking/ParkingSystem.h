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

namespace smart_parking {

/**
 * @struct CameraContext
 * @brief Internal container holding all resources and state for a single camera.
 *
 * This structure groups:
 *  - the processing engine (Parking),
 *  - synchronization primitives (mutex),
 *  - health monitoring data.
 *
 * It is shared across threads via std::shared_ptr and must be accessed
 * under proper locking.
 */
struct CameraContext {
    /**
     * @brief Parking engine responsible for processing frames.
     *
     * Owned exclusively by this context.
     */
    std::unique_ptr<Parking> engine;

    /**
     * @brief Mutex protecting access to this camera context.
     *
     * Ensures thread-safe access to:
     *  - engine
     *  - health flags
     *  - timestamps
     */
    std::mutex mutex;

    /**
     * @brief Timestamp of the last successful frame processing.
     */
    std::chrono::steady_clock::time_point lastUpdate;

    /**
     * @brief Indicates whether the camera is considered healthy.
     *
     * Set to false after repeated processing failures.
     */
    bool healthy;

    /**
     * @brief Number of consecutive processing errors.
     *
     * Used to detect unstable cameras.
     */
    int errorCount;
};

/**
 * @class ParkingSystem
 * @brief Multi-camera manager for parking occupancy systems.
 *
 * This class orchestrates multiple independent Parking instances,
 * each associated with a camera.
 *
 * Responsibilities:
 *  - Manage lifecycle of cameras (add/remove/restart)
 *  - Route frames to the correct processing engine
 *  - Ensure thread-safe access across multiple cameras
 *  - Provide monitoring and statistics
 *
 * Threading model:
 *  - Global mutex (_mutex) protects camera registry
 *  - Per-camera mutex (CameraContext::mutex) protects individual cameras
 *
 * This design minimizes contention and allows parallel processing
 * across multiple cameras.
 */
class ParkingSystem {
public:
    ParkingSystem() = default;

    //
    // --- Camera management ---
    //

    /**
     * @brief Adds a new camera to the system.
     *
     * Creates a new Parking engine using the provided configuration.
     *
     * @param id Unique camera identifier.
     * @param jsonPath Path to parking configuration file.
     * @param reference Reference image for alignment.
     *
     * @throws std::runtime_error if camera already exists.
     */
    void addCamera(
        const std::string& id,
        const std::string& jsonPath,
        const cv::Mat& reference
    );

    /**
     * @brief Removes an existing camera from the system.
     *
     * @param id Camera identifier.
     *
     * @throws std::runtime_error if camera not found.
     */
    void removeCamera(const std::string& id);

    /**
     * @brief Restarts a camera by recreating its processing engine.
     *
     * Useful for recovering from persistent failures.
     *
     * @param id Camera identifier.
     * @param jsonPath Configuration file.
     * @param reference Reference image.
     *
     * @throws std::runtime_error if camera not found.
     */
    void restartCamera(
        const std::string& id,
        const std::string& jsonPath,
        const cv::Mat& reference
    );

    /**
     * @brief Lists all registered camera IDs.
     *
     * @return Vector of camera identifiers.
     */
    std::vector<std::string> listCameras() const;

    //
    // --- Processing ---
    //

    /**
     * @brief Processes a new frame for a given camera.
     *
     * This method:
     *  - retrieves the camera context,
     *  - runs the Parking pipeline,
     *  - updates health monitoring.
     *
     * @param id Camera identifier.
     * @param frame Input frame.
     *
     * @throws std::runtime_error if camera not found.
     * @throws any exception propagated from Parking::evolve().
     */
    void processFrame(
        const std::string& id,
        const Frame& frame
    );

    //
    // --- Retrieval ---
    //

    /**
     * @brief Returns a render snapshot for a given camera.
     *
     * @param id Camera identifier.
     * @return RenderSnapshot containing all visualization data.
     */
    RenderSnapshot getSnapshot(const std::string& id) const;

    /**
     * @brief Returns system statistics for a given camera.
     *
     * @param id Camera identifier.
     * @return Map of metrics (fps, latency, occupancy...).
     */
    std::map<std::string, double> getStats(const std::string& id) const;

    //
    // --- Monitoring ---
    //

    /**
     * @brief Indicates whether a camera is healthy.
     *
     * @param id Camera identifier.
     * @return True if camera is operational.
     */
    bool isHealthy(const std::string& id) const;

    /**
     * @brief Returns the current state of the camera pipeline.
     *
     * @param id Camera identifier.
     * @return CameraState enum.
     */
    CameraState getState(const std::string& id) const;

    /**
     * @brief Returns a string representation of the camera state.
     *
     * @param id Camera identifier.
     * @return State as string.
     */
    std::string getStateString(const std::string& id) const;

    /**
     * @brief Returns timestamp of last successful update.
     */
    std::chrono::steady_clock::time_point
    getLastUpdate(const std::string& id) const;

    /**
     * @brief Returns time elapsed since last update (seconds).
     */
    double getLastUpdateSeconds(const std::string& id) const;

private:
    /**
     * @brief Map of camera ID to camera context.
     *
     * Shared pointers allow safe access across threads.
     */
    std::unordered_map<std::string, std::shared_ptr<CameraContext>> _cameras;

    /**
     * @brief Mutex protecting the camera registry.
     *
     * Must be locked before accessing _cameras.
     */
    mutable std::mutex _mutex;
};

} // namespace smart_parking