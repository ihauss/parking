#pragma once

#include <opencv2/opencv.hpp>
#include <array>

#include "smart_parking/ParkingPlace.h"

namespace smart_parking {

/**
 * @struct RenderPlace
 * @brief Lightweight rendering descriptor for a parking place.
 *
 * This structure is a read-only view used by the rendering system.
 * It decouples visualization from detection and decision logic.
 *
 * It does NOT own any processing logic and does NOT update state.
 */
struct RenderPlace {
    /// Polygon coordinates of the parking place (image space)
    /// Must contain exactly 4 points (quadrilateral)
    std::array<cv::Point, 4> coords;

    /// Current occupancy state of the parking place
    PlaceState state;

    /**
     * @brief Construct a RenderPlace
     */
    RenderPlace(const std::array<cv::Point, 4>& c,
                PlaceState s)
        : coords(c), state(s) {}
};

} // namespace smart_parking