#include "smart_parking/Aligner.h"

namespace smart_parking {

// Constructor: initializes the aligner with a reference frame
Aligner::Aligner(const cv::Mat& reference)
    : _reference(reference.clone()) {
    initReference();
    Logger::log().info("Aligner initialized with reference frame");
}

// Abort if reference tracking has not been initialized
bool Aligner::operator()(const cv::Mat& frame, cv::Mat& warped){
    // Abort if reference tracking has not been initialized
    if (!_flowInitialized)
        return false;

    // Convert current frame to grayscale for optical flow computation
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    // Containers for tracked points, tracking status, and error metrics
    std::vector<cv::Point2f> currPts;
    std::vector<uchar> status;
    std::vector<float> err;

    if (_prevPts.empty()) {
        Logger::log().warn("No tracking points available, skipping alignment.");
        return false;
    }

    // Track reference points from previous frame to current frame
    cv::calcOpticalFlowPyrLK(
        _prevGray,
        gray,
        _prevPts,
        currPts,
        status,
        err
    );

    // Containers for valid point correspondences
    std::vector<cv::Point2f> src, dst;

    // Filter out points that were not successfully tracked
    for (size_t i = 0; i < status.size(); ++i) {
        if (status[i]) {

            // Points in the current frame
            src.push_back(currPts[i]); 

            // Corresponding points in the reference frame
            dst.push_back(_refPts[i]); 
        }
    }

    // Abort if not enough valid correspondences are available
    if (src.size() < 10) {
        Logger::log().warn("Too few tracked points, reinitializing reference");
        initReference();
        return false;
    }

    std::vector<uchar> inliers;
    cv::Mat A = cv::estimateAffinePartial2D(src, dst, inliers, cv::RANSAC);

    if (A.empty()) {
        _affineValid = false;
        return false;
    }

    // Filter with inliers
    std::vector<cv::Point2f> filteredPrev, filteredRef;

    for (size_t i = 0; i < inliers.size(); ++i) {
        if (inliers[i]) {
            filteredPrev.push_back(src[i]);
            filteredRef.push_back(dst[i]);
        }
    }

    _prevPts = filteredPrev;
    _refPts  = filteredRef;

    _lastAffine = A.clone();
    _affineValid = true;

    cv::warpAffine(frame, warped, A, _reference.size());
    _prevGray = gray.clone();

    // Update tracked points for next optical flow computation
    _prevPts  = src;

    return true;
}

// Initializes reference frame tracking data
void Aligner::initReference()
{
    // Convert reference image to grayscale
    cv::Mat gray;
    cv::cvtColor(_reference, gray, cv::COLOR_BGR2GRAY);

    // Detect strong corners to track
    cv::goodFeaturesToTrack(
        gray,
        _refPts,
        500,   // maximum number of corners
        0.01,  // minimum accepted quality of corners
        10     // minimum distance between points
    );

    // Initialize previous grayscale frame
    _prevGray = gray.clone();

    // Initialize previous points for optical flow tracking
    _prevPts  = _refPts;

    // Mark optical flow system as ready
    _flowInitialized = true;

    if (_refPts.empty()) {
        Logger::log().warn("Aligner initialized with 0 reference points");
    }
    else{
        Logger::log().info(
            "Aligner reference initialized with " + std::to_string(_refPts.size()) + " points"
        );
    }
}

bool Aligner::hasAffine() const noexcept {
    return _affineValid;
}

cv::Mat Aligner::getAffine() const {
    return _lastAffine;
}

} // namespace smart_parking