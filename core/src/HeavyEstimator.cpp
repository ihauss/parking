#include "smart_parking/HeavyEstimator.h"

// Default constructor
HeavyEstimator::HeavyEstimator(){}

// Applies a perspective warp to normalize the parking place region
cv::Mat HeavyEstimator::wrap(const cv::Mat& frame, const std::array<cv::Point, 4>& coords){
    // Estimate warped image width using top edge length
    float W = static_cast<int>(cv::norm(coords[0] - coords[1]));

    // Estimate warped image height using left edge length
    float H = static_cast<int>(cv::norm(coords[0] - coords[3]));

    // Source points: original parking polygon corners
    std::vector<cv::Point2f> srcPts = {
        cv::Point2f(coords[0]),
        cv::Point2f(coords[1]),
        cv::Point2f(coords[2]),
        cv::Point2f(coords[3])
    };

    // Destination points: normalized rectangular space
    std::vector<cv::Point2f> dstPts = {
        {0.f, 0.f},
        {static_cast<float>(W - 1), 0.f},
        {static_cast<float>(W - 1), static_cast<float>(H - 1)},
        {0.f, static_cast<float>(H - 1)}
    };

    // Compute perspective transformation matrix
    cv::Mat h = cv::getPerspectiveTransform(srcPts, dstPts);

    // Output warped image
    cv::Mat warped;

    // Apply perspective warp to extract normalized parking region
    cv::warpPerspective(
        frame,
        warped,
        h,
        cv::Size(W, H),
        cv::INTER_LINEAR,
        cv::BORDER_CONSTANT
    );

    return warped;
}

// Estimates occupancy based on luminance variance
bool HeavyEstimator::isOccupied(const cv::Mat& wraped){
    // Convert normalized region to YCrCb color space
    cv::Mat ycrcb;
    cv::cvtColor(wraped, ycrcb, cv::COLOR_BGR2YCrCb);

    // Split YCrCb channels
    std::vector<cv::Mat> channels;
    cv::split(ycrcb, channels);

    // Extract luminance (Y) channel
    cv::Mat Y = channels[0];

    // Compute mean and standard deviation of luminance
    cv::Scalar mean, stddev;
    cv::meanStdDev(Y, mean, stddev);

    // Variance of luminance used as texture indicator
    double varianceY = stddev[0] * stddev[0];

    // High variance indicates presence of a vehicle
    if(varianceY > 1500) return true;

    return false;
}

// Full occupancy estimation pipeline
bool HeavyEstimator::operator()(const cv::Mat& frame, const std::array<cv::Point, 4>& coords){
    // Normalize parking region geometry
    cv::Mat wrapped = wrap(frame, coords);

    // Infer occupancy from appearance statistics
    return isOccupied(wrapped);
}
