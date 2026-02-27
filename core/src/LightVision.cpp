#include "smart_parking/LightVision.h"

// Constructor initializing background subtractor with warm-up phase
LightVision::LightVision(){
    _knn = cv::createBackgroundSubtractorKNN(_warmUp);
}

// Builds a binary mask corresponding to the parking polygon
void LightVision::initMask(const std::array<cv::Point, 4>& coords){
    // Compute bounding box enclosing the parking polygon
    cv::Rect bbox = cv::boundingRect(std::vector<cv::Point>(coords.begin(), coords.end()));
    if (bbox.width <= 0 || bbox.height <= 0) {
        Logger::log().error("LightVision::initMask - invalid bounding box");
        _hasMask = false;
        return;
    }

    // Convert polygon points to local coordinates inside the bounding box
    std::vector<cv::Point> polyLocal;
    for (int i = 0; i < 4; ++i) {
        polyLocal.emplace_back(coords[i].x - bbox.x, coords[i].y - bbox.y);
    }
    
    // Initialize empty binary mask
    _mask = cv::Mat::zeros(bbox.height, bbox.width, CV_8UC1);

    // Fill polygon area with foreground value
    cv::fillPoly(_mask, std::vector<std::vector<cv::Point>>{polyLocal}, 255);
    if (cv::countNonZero(_mask) == 0) {
        Logger::log().error("LightVision::initMask - empty mask");
        _hasMask = false;
        return;
    }
    _hasMask = true;
}

// Computes motion activity inside the parking area
bool LightVision::computeMotionSignal(const cv::Mat& frame, const std::array<cv::Point, 4>& coords, double thresh){
    // Initialize parking mask on first use
    if(!_hasMask) initMask(coords);

    // Extract region of interest corresponding to the parking area
    cv::Rect bbox = cv::boundingRect(std::vector<cv::Point>(coords.begin(), coords.end()));
    if (bbox.x < 0 || bbox.y < 0 ||
        bbox.x + bbox.width  > frame.cols ||
        bbox.y + bbox.height > frame.rows)
    {
        return false;
    }
    cv::Mat roi = frame(bbox);

    // Foreground mask from background subtraction
    cv::Mat fgMask;
    _knn->apply(roi, fgMask);

    // Restrict motion detection to the parking polygon
    cv::bitwise_and(fgMask, _mask, fgMask);

    // Remove noise using morphological opening
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(fgMask, fgMask, cv::MORPH_OPEN, kernel);

    // Compute ratio of moving pixels inside the parking area
    double maskArea = cv::countNonZero(_mask);
    if (maskArea == 0)
        return false;
    double motionRatio = cv::countNonZero(fgMask) / (double)maskArea;

    // Ignore motion during background model warm-up
    if(_warmUp){
        _warmUp--;
        return false;
    }

    // Motion detected if ratio exceeds threshold
    return motionRatio > thresh;
}

// High-level motion estimation with timestamp
LightVisionData LightVision::operator()(const cv::Mat& frame, const std::array<cv::Point, 4>& coords, double thresh){
    // Compute binary motion signal
    bool motion = computeMotionSignal(frame, coords, thresh);

    // Capture current timestamp
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    // Package motion state and timing information
    struct LightVisionData data = {motion, timestamp};
    return data;
}
