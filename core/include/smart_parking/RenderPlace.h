#pragma once

#include <opencv2/opencv.hpp>
#include "smart_parking/ParkingPlace.h"

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
    std::array<cv::Point, 4> coords;

    /// Current occupancy state of the parking place
    PlaceState state;
};
