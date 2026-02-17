#include "smart_parking/Aligner.h"

Aligner::Aligner(cv::Mat reference){
    // Store reference image and initialize tracking
    _reference = reference;
    initReference();
}

bool Aligner::operator()(const cv::Mat& frame, cv::Mat& warped){
    if (!_flowInitialized)
        return false;

    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    std::vector<cv::Point2f> currPts;
    std::vector<uchar> status;
    std::vector<float> err;

    // Track reference points using optical flow
    cv::calcOpticalFlowPyrLK(
        _prevGray,
        gray,
        _prevPts,
        currPts,
        status,
        err
    );

    // Keep only valid tracked points
    std::vector<cv::Point2f> src, dst;
    for (size_t i = 0; i < status.size(); ++i) {
        if (status[i]) {
            src.push_back(currPts[i]);  // current frame points
            dst.push_back(_refPts[i]);  // reference points
        }
    }

    if (src.size() < 10)
        return false;

    // Estimate affine transformation
    std::vector<uchar> inliers;
    cv::Mat A = cv::estimateAffinePartial2D(
        src,
        dst,
        inliers,
        cv::RANSAC
    );

    if (A.empty())
        return false;

    // Warp frame to reference coordinates
    cv::warpAffine(frame, warped, A, _reference.size());

    // Update tracking state
    _prevGray = gray.clone();
    _prevPts  = currPts;

    return true;
}

void Aligner::initReference()
{
    cv::Mat gray;
    cv::cvtColor(_reference, gray, cv::COLOR_BGR2GRAY);

    // Detect strong corners to track
    cv::goodFeaturesToTrack(
        gray,
        _refPts,
        500,   // maximum number of corners
        0.01,  // quality level
        10     // minimum distance between points
    );

    _prevGray = gray.clone();
    _prevPts  = _refPts;
    _flowInitialized = true;
}