#include "smart_parking/LightVision.h"

LightVision::LightVision(){
    _knn = cv::createBackgroundSubtractorKNN(_warmUp);
}

void LightVision::initMask(cv::Point coords[4]){
    cv::Rect bbox = cv::boundingRect(std::vector<cv::Point>(coords, coords + 4));
   
    std::vector<cv::Point> polyLocal;
    for (int i = 0; i < 4; ++i) {
        polyLocal.emplace_back(coords[i].x - bbox.x, coords[i].y - bbox.y);
    }
    
    _mask = cv::Mat::zeros(bbox.height, bbox.width, CV_8UC1);
    cv::fillPoly(_mask, std::vector<std::vector<cv::Point>>{polyLocal}, 255);
}

bool LightVision::hasMovement(const cv::Mat& frame, cv::Point coords[4], double thresh){
    if(!hasMask)initMask(coords);
    cv::Rect bbox = cv::boundingRect(std::vector<cv::Point>(coords, coords + 4));
    cv::Mat roi = frame(bbox);

    cv::Mat fgMask;
    _knn->apply(roi, fgMask);

    cv::bitwise_and(fgMask, _mask, fgMask);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(fgMask, fgMask, cv::MORPH_OPEN, kernel);

    double motionRatio = cv::countNonZero(fgMask) / (double)cv::countNonZero(_mask);

    if(_warmUp){
        _warmUp--;
        return false;
    }

    return motionRatio > thresh;
}

LightVisionData LightVision::operator()(const cv::Mat& frame, cv::Point coords[4], double thresh){
    bool motion = hasMovement(frame, coords, thresh);
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    struct LightVisionData data = {motion, timestamp};
    return data;
}