#include "smart_parking/Aligner.h"

// Constructor: initializes the aligner with a reference frame
Aligner::Aligner(cv::Mat reference){
    // Store reference image used as alignment target
    _reference = reference;

    // Initialize reference features and optical flow state
    initReference();
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
    if (src.size() < 10)
        return false;

    // Container for inlier mask produced by RANSAC
    std::vector<uchar> inliers;

    // Estimate affine transform from current frame to reference frame
    cv::Mat A = cv::estimateAffinePartial2D(
        src,
        dst,
        inliers,
        cv::RANSAC
    );

    // Abort if affine estimation failed
    if (A.empty())
        return false;

    // Warp the current frame into reference coordinate space
    cv::warpAffine(frame, warped, A, _reference.size());

    // Update previous grayscale frame for next iteration
    _prevGray = gray.clone();

    // Update tracked points for next optical flow computation
    _prevPts  = currPts;

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
}