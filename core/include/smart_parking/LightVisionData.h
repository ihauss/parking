#pragma once

#include <chrono>

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
 * Higher-level components (e.g. ParkingPlace, State logic) are responsible
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
     * Uses a monotonic clock to ensure consistency when computing
     * durations or temporal correlations, regardless of system clock changes.
     */
    std::chrono::steady_clock::time_point timestamp{
        std::chrono::steady_clock::now()
    };
};
