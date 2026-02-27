#pragma once

#include <opencv2/opencv.hpp>
#include <array>

#include "smart_parking/LightVisionData.h"
#include "smart_parking/Logger.h"

/**
 * @class LightVision
 * @brief Low-cost visual signal extractor for a single parking place.
 *
 * This class implements lightweight computer vision primitives intended
 * to be executed at high frequency.
 *
 * LightVision is responsible for extracting *signals* (e.g. motion)
 * from the parking place region, but it does NOT:
 *  - decide occupancy state,
 *  - manage temporal consistency,
 *  - trigger state transitions.
 *
 * Its outputs are consumed by higher-level logic (e.g. ParkingPlace)
 * which decides how and when to act upon them.
 */
class LightVision {
private:
    /** Binary mask restricting analysis to the parking place region */
    cv::Mat _mask;

    /** Indicates whether the parking mask has already been initialized */
    bool _hasMask = false;

    /** Background subtractor used for motion detection */
    cv::Ptr<cv::BackgroundSubtractorKNN> _knn;

    /** Number of frames used to stabilize the background model */
    unsigned int _warmUp = 50;

    /**
     * @brief Initializes the binary mask corresponding to the parking polygon.
     *
     * This method creates a binary mask from the provided parking place
     * coordinates. The mask is later used to restrict motion detection
     * strictly to the parking area.
     *
     * This method is expected to be called once per parking place,
     * typically on the first processed frame.
     *
     * @param coords Four corner points defining the parking place region
     *               in the source frame.
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
     * This method applies background subtraction on the input frame
     * and measures the proportion of foreground pixels inside
     * the parking mask.
     *
     * The resulting motion ratio is compared against a threshold
     * to produce a boolean motion signal.
     *
     * @param frame Current video frame.
     * @param coords Parking place corner coordinates.
     * @param thresh Motion ratio threshold.
     *
     * @return True if motion exceeds the given threshold, false otherwise.
     */
    bool computeMotionSignal(
        const cv::Mat& frame,
        const std::array<cv::Point, 4>& coords,
        double thresh
    );

    /**
     * @brief Computes lightweight visual signals for the parking place.
     *
     * This call operator aggregates all low-cost visual computations
     * (e.g. motion detection) into a single LightVisionData structure.
     *
     * It is intended to be called at high frequency as part of
     * the real-time processing loop.
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
