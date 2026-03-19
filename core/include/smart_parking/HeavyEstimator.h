#pragma once

#include <opencv2/opencv.hpp>
#include <array>

#include "smart_parking/Logger.h"

namespace smart_parking {

/**
 * @class HeavyEstimator
 * @brief High-cost visual occupancy estimator for a single parking place.
 */
class HeavyEstimator {
public:
    HeavyEstimator();

    /**
     * @brief Warp parking region into canonical view.
     *
     * Allocates a new image (costly operation)
     *
     * @param frame Must be non-empty (CV_8UC3)
     * @param coords Must define a valid quadrilateral
     *
     * @return Warped parking image
     */
    cv::Mat warp(
        const cv::Mat& frame,
        const std::array<cv::Point, 4>& coords
    ) const;

    /**
     * @brief Estimate occupancy from warped image
     */
    bool isOccupied(const cv::Mat& warped) const;

    /**
     * @brief Full pipeline: warp + occupancy estimation
     *
     * Does not expose intermediate warped image
     */
    bool operator()(
        const cv::Mat& frame,
        const std::array<cv::Point, 4>& coords
    ) const;
};

} // namespace smart_parking