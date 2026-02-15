#ifndef LIGHT_VISION_H
#define LIGHT_VISION_H

#include <opencv2/opencv.hpp>

class LightVision {
private:
    cv::Mat _mask;
    bool hasMask = false;
    cv::Ptr<cv::BackgroundSubtractorKNN> _knn;
    unsigned int _warmUp = 50;

    /**
     * @brief Initializes the binary mask corresponding to the parking polygon.
     *
     * The mask is used to restrict motion detection and appearance analysis
     * strictly to the parking area.
     */
    void initMask(cv::Point coords[4]);
public:
    LightVision();

    /**
     * @brief Detects motion inside the parking place region.
     *
     * Motion detection is performed using a KNN background subtractor
     * restricted to the parking mask.
     *
     * @param frame Current video frame.
     * @param thresh Motion ratio threshold.
     * @return True if motion exceeds the given threshold.
     */
    bool hasMovement(const cv::Mat& frame, cv::Point coords[4], double thresh);
};

#endif