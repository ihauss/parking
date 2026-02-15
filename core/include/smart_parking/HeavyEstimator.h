#ifndef HEAVY_ESTIMATOR_H
#define HEAVY_ESTIMATOR_H

#include <opencv2/opencv.hpp>

class HeavyEstimator{
private:
public:
    HeavyEstimator();

    /**
     * @brief Evaluates the occupancy state of the parking place.
     *
     * The method applies a perspective warp to normalize the parking region,
     * converts it to YCrCb color space, and estimates luminance variance
     * to infer whether the place is occupied.
     *
     * @param frame Current video frame.
     * @return Estimated occupancy state.
     */

    cv::Mat wrap(cv::Mat& frame, cv::Point coords[4]);

    bool isOccupied(cv::Mat& wraped);

    bool operator()(cv::Mat& frame, cv::Point coords[4]);
};

#endif