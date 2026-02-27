#pragma once

#include <opencv2/opencv.hpp>
#include <array>

#include "smart_parking/Logger.h"

/**
 * @class HeavyEstimator
 * @brief High-cost visual occupancy estimator for a single parking place.
 *
 * This class implements a robust but computationally expensive method
 * to estimate whether a parking place is occupied.
 *
 * It operates exclusively on image data and geometric configuration
 * (parking place coordinates) and does NOT:
 *  - decide when it should be executed,
 *  - manage temporal logic,
 *  - trigger state transitions.
 *
 * The HeavyEstimator is intended to be invoked sparingly by higher-level
 * orchestration logic (e.g. ParkingPlace) when a reliable confirmation
 * is required.
 */
class HeavyEstimator {
public:
    /**
     * @brief Constructs a HeavyEstimator instance.
     *
     * No heavy resources are allocated at construction time.
     */
    HeavyEstimator();

    /**
     * @brief Applies a perspective warp to normalize the parking area.
     *
     * This method extracts the parking place region from the input frame
     * using the provided corner coordinates and warps it into a canonical
     * top-down view.
     *
     * The resulting image is intended to be used as input for
     * occupancy analysis.
     *
     * @param frame Source video frame.
     * @param coords Four corner points defining the parking place region
     *               in the source frame (clockwise or counter-clockwise).
     *
     * @return Warped image of the parking place.
     */
    cv::Mat wrap(
        const cv::Mat& frame,
        const std::array<cv::Point, 4>& coords
    );

    /**
     * @brief Estimates whether the parking place is occupied.
     *
     * This method performs a computationally intensive analysis on the
     * normalized parking place image. The exact implementation may include:
     *  - color space conversion,
     *  - luminance or chrominance statistics,
     *  - variance or texture analysis.
     *
     * The input image is assumed to be already warped and normalized.
     *
     * @param warped Normalized parking place image.
     *
     * @return True if the parking place is considered occupied, false otherwise.
     */
    bool isOccupied(const cv::Mat& warped);

    /**
     * @brief Convenience call operator combining warp and occupancy estimation.
     *
     * This operator performs the full heavy estimation pipeline:
     *  1. warp the parking place region,
     *  2. evaluate its occupancy state.
     *
     * It is provided as a shorthand for callers that do not need
     * intermediate results.
     *
     * @param frame Source video frame.
     * @param coords Parking place corner coordinates.
     *
     * @return True if the parking place is considered occupied, false otherwise.
     */
    bool operator()(
        const cv::Mat& frame,
        const std::array<cv::Point, 4>& coords
    );
};
