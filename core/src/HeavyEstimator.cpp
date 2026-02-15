#include "smart_parking/HeavyEstimator.h"

HeavyEstimator::HeavyEstimator(){}

cv::Mat HeavyEstimator::wrap(cv::Mat& frame, cv::Point coords[4]){
    float W = static_cast<int>(cv::norm(coords[0] - coords[1]));
    float H = static_cast<int>(cv::norm(coords[0] - coords[3]));

    std::vector<cv::Point2f> srcPts = {
        cv::Point2f(coords[0]),
        cv::Point2f(coords[1]),
        cv::Point2f(coords[2]),
        cv::Point2f(coords[3])
    };

    std::vector<cv::Point2f> dstPts = {
        {0.f, 0.f},
        {static_cast<float>(W - 1), 0.f},
        {static_cast<float>(W - 1), static_cast<float>(H - 1)},
        {0.f, static_cast<float>(H - 1)}
    };

    cv::Mat h = cv::getPerspectiveTransform(srcPts, dstPts);

    cv::Mat warped;
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

bool HeavyEstimator::isOccupied(cv::Mat& wraped){
    cv::Mat ycrcb;
    cv::cvtColor(wraped, ycrcb, cv::COLOR_BGR2YCrCb);

    std::vector<cv::Mat> channels;
    cv::split(ycrcb, channels);

    cv::Mat Y = channels[0];

    // variance
    cv::Scalar mean, stddev;
    cv::meanStdDev(Y, mean, stddev);
    double varianceY = stddev[0] * stddev[0];

    if(varianceY > 1500)return true;

    return false;
}

bool HeavyEstimator::operator()(cv::Mat& frame, cv::Point coords[4]){
    cv::Mat wrapped = wrap(frame, coords);
    return isOccupied(wrapped);
}