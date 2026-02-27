#pragma once

#include <opencv2/opencv.hpp>

#include "smart_parking/Logger.h"

/**
 * @class Aligner
 * @brief Frame-to-reference image alignment using optical flow.
 *
 * This class compensates for small camera motions by aligning incoming frames
 * to a fixed reference image. It relies on sparse optical flow tracking
 * and affine transformation estimation.
 *
 * The reference image is assumed to be static and representative of the scene.
 * The class maintains internal temporal state (previous frame and features)
 * to ensure stable optical flow tracking over time.
 *
 * Responsibilities:
 *  - Track visual features across frames
 *  - Estimate inter-frame motion
 *  - Warp frames to match the reference coordinate system
 *
 * Non-responsibilities:
 *  - Does NOT decide when alignment is needed
 *  - Does NOT manage frame acquisition
 */
class Aligner {
private:
    /// Reference image used as alignment target
    cv::Mat _reference;

    /// Previous grayscale frame (used for optical flow)
    cv::Mat _prevGray;

    /// Feature points tracked in the previous frame
    std::vector<cv::Point2f> _prevPts;

    /// Feature points detected in the reference image
    std::vector<cv::Point2f> _refPts;

    /// Indicates whether optical flow tracking has been initialized
    bool _flowInitialized = false;

    cv::Mat _lastAffine;
    bool _affineValid = false;

    /**
     * @brief Initializes feature points on the reference image.
     *
     * Detects stable and well-distributed feature points that will be used
     * as anchors for optical flow tracking.
     *
     * This method must be called before any alignment attempt.
     */
    void initReference();

public:
    /**
     * @brief Constructs an Aligner with a given reference image.
     *
     * @param reference Reference image defining the target coordinate system.
     */
    Aligner(cv::Mat reference);

    /**
     * @brief Aligns an input frame to the reference image.
     *
     * Estimates camera motion using optical flow and computes an affine
     * transformation to warp the frame into the reference coordinate space.
     *
     * @param frame Input frame to align (unchanged).
     * @param warped Output aligned frame.
     *
     * @return True if alignment was successful, false otherwise
     *         (e.g. insufficient tracked points or unstable estimation).
     */
    bool operator()(const cv::Mat& frame, cv::Mat& warped);

    bool hasAffine() const;
    cv::Mat getAffine() const;
};
