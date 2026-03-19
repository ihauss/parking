#include "smart_parking/LightVision.h"

namespace smart_parking {

// Constructor initializing background subtractor with warm-up phase
LightVision::LightVision(){
    // Initialize KNN background subtractor
    // _warmUp is used here as history length (number of frames)
    _knn = cv::createBackgroundSubtractorKNN(_warmUp);

    Logger::log().info(
        "LightVision initialized with warmup=" + std::to_string(_warmUp)
    );
}

// Builds a binary mask corresponding to the parking polygon
void LightVision::initMask(const std::array<cv::Point, 4>& coords){
    Logger::log().info("LightVision::initMask - initializing mask");

    // Compute bounding box enclosing the parking polygon
    cv::Rect bbox = cv::boundingRect(std::vector<cv::Point>(coords.begin(), coords.end()));

    // Validate bounding box
    if (bbox.width <= 0 || bbox.height <= 0) {
        Logger::log().error("LightVision::initMask - invalid bounding box");
        _hasMask = false;
        return;
    }

    // Convert polygon points to local coordinates inside the bounding box
    // This allows us to build a mask relative to the ROI instead of full frame
    std::vector<cv::Point> polyLocal;
    polyLocal.reserve(4); // avoid reallocations

    for (int i = 0; i < 4; ++i) {
        polyLocal.emplace_back(coords[i].x - bbox.x, coords[i].y - bbox.y);
    }
    
    // Initialize empty binary mask (same size as ROI)
    _mask = cv::Mat::zeros(bbox.height, bbox.width, CV_8UC1);

    // Fill polygon area with foreground value (255)
    cv::fillPoly(_mask, std::vector<std::vector<cv::Point>>{polyLocal}, 255);

    // Validate mask content
    if (cv::countNonZero(_mask) == 0) {
        Logger::log().error("LightVision::initMask - empty mask");
        _hasMask = false;
        return;
    }

    _hasMask = true;

    Logger::log().info(
        "LightVision::initMask - mask initialized (area=" +
        std::to_string(cv::countNonZero(_mask)) + ")"
    );
}

// Computes motion activity inside the parking area
bool LightVision::computeMotionSignal(
    const cv::Mat& frame,
    const std::array<cv::Point, 4>& coords,
    double thresh
){
    // Initialize parking mask on first use
    if(!_hasMask) {
        initMask(coords);

        // Safety: if mask creation failed, abort early
        if(!_hasMask) {
            Logger::log().warn(
                "LightVision::computeMotionSignal - mask initialization failed"
            );
            return false;
        }
    }

    // Compute bounding box again (cheap and avoids storing stale state)
    cv::Rect bbox = cv::boundingRect(std::vector<cv::Point>(coords.begin(), coords.end()));

    // Validate ROI boundaries to prevent OpenCV crash
    if (bbox.x < 0 || bbox.y < 0 ||
        bbox.x + bbox.width  > frame.cols ||
        bbox.y + bbox.height > frame.rows)
    {
        Logger::log().warn(
            "LightVision::computeMotionSignal - ROI out of bounds"
        );
        return false;
    }

    // Extract region of interest corresponding to the parking area
    cv::Mat roi = frame(bbox);

    // Foreground mask from background subtraction
    cv::Mat fgMask;
    _knn->apply(roi, fgMask);

    // Restrict motion detection to the parking polygon
    cv::bitwise_and(fgMask, _mask, fgMask);

    // Remove noise using morphological opening
    static const cv::Mat kernel =
        cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));

    cv::morphologyEx(fgMask, fgMask, cv::MORPH_OPEN, kernel);

    // Compute ratio of moving pixels inside the parking area
    double maskArea = cv::countNonZero(_mask);

    if (maskArea == 0) {
        Logger::log().warn(
            "LightVision::computeMotionSignal - mask area is zero"
        );
        return false;
    }

    double motionRatio = cv::countNonZero(fgMask) / maskArea;

    // Debug-level useful info (can downgrade later if too verbose)
    Logger::log().info(
        "LightVision::computeMotionSignal - motionRatio=" +
        std::to_string(motionRatio)
    );

    // Ignore motion during background model warm-up
    if(_warmUp > 0){
        _warmUp--;

        Logger::log().info(
            "LightVision warmup... remaining=" + std::to_string(_warmUp)
        );

        return false;
    }

    // Motion detected if ratio exceeds threshold
    bool detected = motionRatio > thresh;

    if (detected) {
        Logger::log().info(
            "LightVision::computeMotionSignal - motion detected"
        );
    }

    return detected;
}

// High-level motion estimation with timestamp
LightVisionData LightVision::operator()(
    const cv::Mat& frame,
    const std::array<cv::Point, 4>& coords,
    double thresh
){
    // Compute binary motion signal
    bool motion = computeMotionSignal(frame, coords, thresh);

    // Capture current timestamp
    std::chrono::steady_clock::time_point timestamp =
        std::chrono::steady_clock::now();

    // Package motion state and timing information
    LightVisionData data{motion, timestamp};

    return data;
}

} // namespace smart_parking