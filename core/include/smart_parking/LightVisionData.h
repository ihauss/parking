#pragma once

#include <chrono>

namespace smart_parking {

/**
 * @struct LightVisionData
 * @brief Lightweight visual signals extracted from a parking place.
 *
 * This structure encapsulates the output of the LightVision module.
 * It represents instantaneous visual observations and does NOT encode
 * any decision, state, or temporal aggregation logic.
 *
 * LightVisionData is designed to be:
 *  - cheap to compute,
 *  - cheap to copy,
 *  - easy to expose through an API or bindings.
 *
 * Higher-level components (e.g. ParkingPlace, state logic) are responsible
 * for interpreting these signals over time.
 */
struct LightVisionData
{
    /**
     * @brief Indicates whether motion was detected inside the parking place.
     *
     * This flag is typically derived from background subtraction and
     * thresholding. It does not imply occupancy by itself.
     */
    bool hasMovement{false};

    /**
     * @brief Timestamp associated with the extracted visual signals.
     *
     * Must be provided by the producer to ensure temporal consistency.
     * Uses a monotonic clock to allow reliable duration computations.
     */
    std::chrono::steady_clock::time_point timestamp;

    /**
     * @brief Construct a LightVisionData instance.
     *
     * @param movement Motion flag
     * @param ts Timestamp of the observation
     */
    LightVisionData(bool movement,
                    std::chrono::steady_clock::time_point ts)
        : hasMovement(movement), timestamp(ts)
    {}
};

} // namespace smart_parking