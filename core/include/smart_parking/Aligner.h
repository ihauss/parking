#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

#include "smart_parking/Logger.h"

namespace smart_parking {

/**
 * @class Aligner
 * @brief Frame-to-reference image alignment using optical flow.
 *
 * Not thread-safe. One instance per camera must be used.
 */
class Aligner {
private:
    /// Reference image (immutable)
    const cv::Mat _reference;

    /// Previous grayscale frame
    cv::Mat _prevGray;

    /// Previously tracked feature points
    std::vector<cv::Point2f> _prevPts;

    /// Reference feature points
    std::vector<cv::Point2f> _refPts;

    /// Optical flow initialization flag
    bool _flowInitialized = false;

    /// Last estimated affine transform (2x3, CV_64F)
    cv::Mat _lastAffine;

    /// Whether last affine is valid
    bool _affineValid = false;

    void initReference();

public:
    /**
     * @brief Construct an Aligner
     * @param reference Reference image (BGR uint8)
     */
    explicit Aligner(const cv::Mat& reference);

    /**
     * @brief Align frame to reference
     *
     * @param frame Input frame (BGR uint8, same size as reference)
     * @param warped Output aligned frame
     *
     * @return True if alignment succeeded
     */
    bool operator()(const cv::Mat& frame, cv::Mat& warped);

    bool hasAffine() const noexcept;

    /**
     * @brief Get last affine transform
     * @return Copy of the affine matrix
     */
    cv::Mat getAffine() const;
};

} // namespace smart_parking