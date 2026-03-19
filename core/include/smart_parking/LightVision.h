#pragma once

#include <opencv2/opencv.hpp>
#include <array>

#include "smart_parking/LightVisionData.h"
#include "smart_parking/Logger.h"

namespace smart_parking {

/**
 * @class LightVision
 * @brief Low-cost visual signal extractor for a single parking place.
 *
 * This class implements lightweight computer vision primitives intended
 * to be executed at high frequency (e.g. real-time video processing).
 *
 * LightVision is responsible for extracting *signals* (e.g. motion)
 * from the parking place region, but it does NOT:
 *  - decide occupancy state,
 *  - manage temporal consistency,
 *  - trigger state transitions.
 *
 * Its outputs are consumed by higher-level logic (e.g. ParkingPlace),
 * which decides how and when to act upon them.
 *
 * Design goals:
 *  - Fast execution (runs every frame)
 *  - Minimal allocations after initialization
 *  - Robust to small lighting variations
 *
 * Important:
 *  - This class maintains internal state (background model).
 *  - It is NOT thread-safe.
 */
class LightVision {
private:
    /**
     * @brief Binary mask restricting analysis to the parking place region.
     *
     * This mask is computed once from the parking polygon and reused
     * for all subsequent frames to avoid unnecessary recomputation.
     */
    cv::Mat _mask;

    /**
     * @brief Indicates whether the parking mask has been initialized.
     */
    bool _hasMask = false;

    /**
     * @brief Background subtractor used for motion detection.
     *
     * Uses a KNN-based model to estimate foreground regions.
     * This model is updated at each frame.
     */
    cv::Ptr<cv::BackgroundSubtractorKNN> _knn;

    /**
     * @brief Warm-up parameter.
     *
     * Dual role:
     *  - Used as history length when initializing the KNN subtractor
     *  - Used as a countdown to ignore unstable motion detections
     *
     * This coupling is not ideal and could be split into:
     *  - history length
     *  - runtime warm-up counter
     */
    unsigned int _warmUp = 50;

    /**
     * @brief Initializes the binary mask corresponding to the parking polygon.
     *
     * This method creates a binary mask from the provided parking place
     * coordinates. The mask is used to restrict motion detection strictly
     * to the parking area.
     *
     * This method is called lazily on the first frame.
     *
     * @param coords Four corner points defining the parking place region.
     */
    void initMask(const std::array<cv::Point, 4>& coords);

public:
    /**
     * @brief Constructs a LightVision instance.
     *
     * Initializes the background subtractor but does not allocate
     * any parking-specific resources (mask).
     */
    LightVision();

    /**
     * @brief Computes a motion signal inside the parking place region.
     *
     * This method:
     *  - applies background subtraction,
     *  - restricts the result to the parking mask,
     *  - computes the ratio of moving pixels,
     *  - compares it against a threshold.
     *
     * Important:
     *  - This method updates the internal background model.
     *  - It is stateful and must be called sequentially.
     *
     * @param frame Current video frame (must be non-empty, CV_8UC3).
     * @param coords Parking place corner coordinates.
     * @param thresh Motion ratio threshold (e.g. 0.05).
     *
     * @return True if motion exceeds the threshold, false otherwise.
     */
    bool computeMotionSignal(
        const cv::Mat& frame,
        const std::array<cv::Point, 4>& coords,
        double thresh
    );

    /**
     * @brief Computes lightweight visual signals for the parking place.
     *
     * This operator aggregates all low-cost visual computations
     * (currently motion detection) into a single LightVisionData structure.
     *
     * It is intended to be called at high frequency in a real-time loop.
     *
     * Internally:
     *  - handles mask initialization,
     *  - handles warm-up phase,
     *  - computes motion signal.
     *
     * @param frame Current video frame.
     * @param coords Parking place corner coordinates.
     * @param thresh Motion ratio threshold.
     *
     * @return A LightVisionData structure containing extracted signals.
     */
    LightVisionData operator()(
        const cv::Mat& frame,
        const std::array<cv::Point, 4>& coords,
        double thresh
    );
};

} // namespace smart_parking