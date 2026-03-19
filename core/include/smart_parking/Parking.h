#pragma once

#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <stdexcept>
#include <map>

#include "smart_parking/ParkingPlace.h"
#include "smart_parking/Aligner.h"
#include "smart_parking/RenderSnapshot.h"
#include "smart_parking/Logger.h"
#include "smart_parking/Frame.h"
#include "smart_parking/CameraState.h"

namespace smart_parking {

/**
 * @class Parking
 * @brief High-level orchestrator for parking occupancy estimation.
 *
 * The Parking class represents a complete parking area composed of multiple
 * ParkingPlace instances. It is responsible for coordinating the full
 * perception pipeline:
 *
 *  - loading parking place geometry from a configuration file,
 *  - aligning incoming frames to a fixed reference image,
 *  - updating each parking place (state machine + async estimation),
 *  - aggregating global statistics (occupancy, FPS, latency),
 *  - exposing rendering-friendly data structures.
 *
 * Design principles:
 *  - Delegation: low-level vision is handled by dedicated components
 *    (Aligner, LightVision, HeavyEstimator).
 *  - Real-time: designed to process frames sequentially at high frequency.
 *  - Non-blocking: heavy computations are asynchronous at ParkingPlace level.
 *
 * This class is the main entry point of the system.
 */
class Parking {
private:

    /**
     * @brief Number of currently occupied parking places.
     *
     * Recomputed at each call to evolve() based on the state
     * of all ParkingPlace instances.
     */
    int _numOccupied{0};

    /**
     * @brief Collection of parking places managed by the system.
     */
    std::vector<ParkingPlace> _places;

    /**
     * @brief Last measured end-to-end latency (milliseconds).
     *
     * Includes alignment + state updates for all parking places.
     */
    double _lastLatencyMs{0.0};

    /**
     * @brief Estimated frames per second.
     *
     * Computed from the processing latency or frame timestamps.
     */
    double _fps{0.0};

    /**
     * @brief Frame alignment module.
     *
     * Compensates for camera motion by aligning incoming frames
     * to a reference coordinate system.
     */
    Aligner _aligner;

    /**
     * @brief Current state of the camera / processing pipeline.
     *
     * Used to expose system health (e.g. running, error).
     */
    CameraState _state{CameraState::IDLE};

    /**
     * @brief Total number of processed frames.
     *
     * Useful for statistics, debugging, and monitoring.
     */
    size_t _frameCount{0};

public:

    /**
     * @brief Constructs a Parking manager from configuration.
     *
     * This constructor:
     *  - loads parking place definitions from a JSON file,
     *  - initializes ParkingPlace instances,
     *  - initializes the alignment module with a reference frame.
     *
     * @param jsonPath Path to JSON file describing parking geometry.
     * @param reference Reference image used for alignment.
     *
     * @throws std::runtime_error if file cannot be read or parsed.
     */
    Parking(const std::string& jsonPath, const cv::Mat& reference);

    /**
     * @brief Returns the total number of parking places.
     *
     * @return Number of configured parking slots.
     */
    size_t getNumPlace() const;

    /**
     * @brief Returns the number of currently occupied places.
     *
     * Value is updated during the last evolve() call.
     *
     * @return Number of occupied parking slots.
     */
    int getNumOccupied() const;

    /**
     * @brief Returns the current FPS estimate.
     *
     * Based on recent processing latency.
     *
     * @return Frames per second.
     */
    double getFps() const;

    /**
     * @brief Returns the last measured latency.
     *
     * @return Latency in milliseconds.
     */
    double getLastLatencyMs() const;

    /**
     * @brief Returns aggregated system statistics.
     *
     * Includes:
     *  - FPS,
     *  - latency,
     *  - occupancy ratio.
     *
     * @return Map of metric name → value.
     */
    std::map<std::string, double> getStats() const;

    /**
     * @brief Main update function called for each frame.
     *
     * This method executes the full pipeline:
     *
     *  1. Validate input frame,
     *  2. Align frame to reference (if possible),
     *  3. Update all ParkingPlace instances,
     *  4. Aggregate global statistics.
     *
     * This method must be called sequentially (not thread-safe).
     *
     * @param frame Input frame wrapper (validated BGR image).
     */
    void evolve(const Frame& frame);

    /**
     * @brief Retrieve the minimal information to render a place
     */
    std::vector<RenderPlace> getRenderData() const;

    /**
     * @brief Returns rendering snapshot of the current system state.
     *
     * The snapshot includes:
     *  - parking place geometry and states,
     *  - affine transform (if available),
     *  - global statistics (occupancy).
     *
     * This is the preferred interface for visualization layers.
     *
     * @return RenderSnapshot structure.
     */
    RenderSnapshot getRenderSnapshot() const;

    /**
     * @brief Indicates whether a valid affine transform is available.
     *
     * @return True if alignment succeeded recently.
     */
    bool hasAffine() const;

    /**
     * @brief Returns the last estimated affine transform.
     *
     * The transform is encoded as a 2x3 matrix flattened into
     * an array of 6 values.
     *
     * Must call hasAffine() before using this.
     *
     * @return Affine transform coefficients.
     */
    std::array<double, 6> getAffine() const;

    /**
     * @brief Returns current camera/system state.
     *
     * @return CameraState enum.
     */
    CameraState getState() const;
};

} // namespace smart_parking