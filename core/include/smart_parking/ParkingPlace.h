#pragma once

#include <opencv2/opencv.hpp>
#include <future>
#include <optional>
#include <chrono>

#include "smart_parking/LightVision.h"
#include "smart_parking/HeavyEstimator.h"
#include "smart_parking/PlaceState.h"
#include "smart_parking/Logger.h"

namespace smart_parking {

/**
 * @class ParkingPlace
 * @brief Represents a single parking slot and manages its occupancy state.
 *
 * ParkingPlace encapsulates all logic related to ONE parking spot:
 *
 *  - geometry (polygon coordinates),
 *  - lightweight motion detection (LightVision),
 *  - heavyweight appearance-based occupancy estimation (HeavyEstimator),
 *  - state management (FREE, OCCUPIED, TRANSITION_IN, TRANSITION_OUT).
 *
 * IMPORTANT:
 * - LightVision and HeavyEstimator NEVER decide the final state.
 * - ParkingPlace is the sole owner of the state machine.
 *
 * Heavy computations are executed asynchronously to avoid blocking
 * the main processing loop.
 *
 * Threading:
 * - This class uses std::future for async heavy estimation.
 * - It is NOT thread-safe and must be used from a single thread.
 */
class ParkingPlace {
private:
    /**
     * @brief Unique identifier of the parking place.
     */
    int _id;

    /**
     * @brief Polygon defining the parking slot (4 corners).
     */
    std::array<cv::Point, 4> _coords;

    /**
     * @brief Indicates whether coordinates were clamped to frame boundaries.
     */
    bool _coordAdjust{false};

    /**
     * @brief Lightweight vision module for motion detection.
     *
     * Provides short-term signals (movement, timestamps),
     * but does not infer occupancy.
     */
    LightVision _lightVision;

    /**
     * @brief Heavy appearance-based occupancy estimator.
     *
     * Used only when motion stabilizes or a state transition must be confirmed.
     */
    HeavyEstimator _estimator;

    /**
     * @brief Asynchronous result of a heavy occupancy estimation.
     *
     * Must always be checked with .valid() before use.
     */
    std::future<bool> _occupancyResultAsync;

    /**
     * @brief Cached result of the last completed heavy estimation.
     */
    std::optional<bool> _occupancyResult;

    /**
     * @brief Current logical state of the parking place.
     */
    PlaceState _currentState{PlaceState::INIT_STATE};

    /**
     * @brief Timestamp of the last heavy estimation launch or validation.
     *
     * Used to enforce cooldowns and temporal consistency.
     */
    std::chrono::steady_clock::time_point _lastEstimationTime;

public:
    /**
     * @brief Constructs a ParkingPlace from polygon coordinates.
     *
     * @param coords Array of four points defining the parking slot.
     * @param id Unique identifier of the parking place.
     */
    ParkingPlace(const std::array<cv::Point, 4> coords, int id = 0);

    /**
     * @brief Checks whether a timestamp is recent relative to a cooldown duration.
     *
     * @param timestamp Timestamp to compare.
     * @param coolDownTime Minimum allowed duration.
     * @return True if timestamp is within cooldown window.
     */
    bool isRecent(std::chrono::steady_clock::time_point timestamp,
                  std::chrono::milliseconds coolDownTime);

    /**
     * @brief Updates the internal state machine using vision signals.
     *
     * This method applies the transition rules using:
     *  - LightVisionData (motion signal),
     *  - optional HeavyEstimator result (occupancy confirmation).
     *
     * @param data Output of LightVision (read-only).
     * @param info Optional heavy estimation result.
     * @return True if the state changed.
     */
    bool update(const LightVisionData& data,
                std::optional<bool> info);

    /**
     * @brief Ensures parking coordinates stay inside frame boundaries.
     *
     * If coordinates exceed the frame size, they are clamped and
     * dependent masks are recomputed.
     *
     * @param frameSize Size of the current video frame.
     * @return True if any coordinate was modified.
     */
    bool adjustCoords(const cv::Size& frameSize);

    /**
     * @brief Returns the current parking place state.
     */
    PlaceState getState() const;

    /**
     * @brief Returns the polygon defining the parking place.
     *
     * @return Reference to array of four cv::Point.
     */
    const std::array<cv::Point, 4>& getCoords() const;

    /**
     * @brief Main update entry point called for each new frame.
     *
     * This method:
     *  - computes motion signals,
     *  - launches heavy estimations asynchronously when needed,
     *  - updates the internal state machine accordingly.
     *
     * @param frame Current video frame (read-only).
     */
    void changeState(const cv::Mat& frame);
};

} // namespace smart_parking