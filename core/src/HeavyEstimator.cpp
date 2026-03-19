#include "smart_parking/HeavyEstimator.h"

namespace smart_parking {

HeavyEstimator::HeavyEstimator() {}

cv::Mat HeavyEstimator::warp(const cv::Mat& frame,
                             const std::array<cv::Point, 4>& coords) const
{
    if (frame.empty()) {
        Logger::log().error("HeavyEstimator::warp - empty frame");
        return {};
    }

    float widthA = cv::norm(coords[2] - coords[3]);
    float widthB = cv::norm(coords[1] - coords[0]);
    float W = std::max(widthA, widthB);

    float heightA = cv::norm(coords[1] - coords[2]);
    float heightB = cv::norm(coords[0] - coords[3]);
    float H = std::max(heightA, heightB);

    if (W < 5 || H < 5) {
        Logger::log().error("HeavyEstimator::warp - invalid size");
        return {};
    }

    std::vector<cv::Point2f> srcPts(coords.begin(), coords.end());

    std::vector<cv::Point2f> dstPts = {
        {0.f, 0.f},
        {W - 1, 0.f},
        {W - 1, H - 1},
        {0.f, H - 1}
    };

    cv::Mat h = cv::getPerspectiveTransform(srcPts, dstPts);
    if (h.empty()) {
        Logger::log().error("HeavyEstimator::warp failed");
        return {};
    }

    cv::Mat warped;
    cv::warpPerspective(frame, warped, h, cv::Size(W, H));

    return warped;
}

bool HeavyEstimator::isOccupied(const cv::Mat& warped) const
{
    if (warped.empty()) return false;

    cv::Mat ycrcb;
    cv::cvtColor(warped, ycrcb, cv::COLOR_BGR2YCrCb);

    std::vector<cv::Mat> channels;
    cv::split(ycrcb, channels);

    cv::Mat Y = channels[0];

    cv::GaussianBlur(Y, Y, cv::Size(5,5), 0);

    cv::Scalar mean, stddev;
    cv::meanStdDev(Y, mean, stddev);

    double varianceY = stddev[0] * stddev[0];

    return varianceY > 1500;
}

bool HeavyEstimator::operator()(const cv::Mat& frame,
                                const std::array<cv::Point, 4>& coords) const
{
    cv::Mat warped = warp(frame, coords);

    if (warped.empty()) {
        Logger::log().warn("HeavyEstimator warp failed");
        return false;
    }

    return isOccupied(warped);
}

} // namespace smart_parking
