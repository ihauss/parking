#include "smart_parking/ParkingPlace.h"

const std::array<cv::Scalar, 4> COLORS = {
    cv::Scalar(0, 255, 0),  
    cv::Scalar(0, 127, 255), 
    cv::Scalar(0, 0, 255),   
    cv::Scalar(0, 255, 255)
};

ParkingPlace::ParkingPlace(const cv::Point coords[4], int id)
    : _id(id)
{
    // Copy 4 points
    for (int i = 0; i < 4; i++) {
        _coords[i] = coords[i];
    }
}

placeState ParkingPlace::getState() const {
    return _stateManager.getState();
}

const cv::Point* ParkingPlace::getCoords() const {
    return _coords;
}

bool ParkingPlace::adjustCoords(cv::Size& frameSize)
{
    bool modified = false;

    for (int i = 0; i < 4; ++i) {
        int x = _coords[i].x;
        int y = _coords[i].y;

        int clampedX = std::clamp(x, 0, frameSize.width  - 1);
        int clampedY = std::clamp(y, 0, frameSize.height - 1);

        if (x != clampedX || y != clampedY) {
            _coords[i].x = clampedX;
            _coords[i].y = clampedY;
            modified = true;
        }
    }
    _coordAdjust = true;

    return modified;
}

void ParkingPlace::changeState(cv::Mat& frame) {
    cv::Size frameSize = frame.size();
    if(!_coordAdjust)adjustCoords(frameSize);
    struct LightVisionData data = _lightVision(frame, _coords, 0.25);
    _stateManager(data, frame, _coords);
}

void ParkingPlace::drawPlace(cv::Mat& frame, cv::Mat& overlay){
    std::vector<std::vector<cv::Point>> contours;
    contours.emplace_back(_coords, _coords + 4);
    cv::Scalar color = COLORS[_stateManager.getState()-1];
    int thickness = 3;
    cv::polylines(frame, contours, true, color, thickness, cv::LINE_AA);
    cv::fillPoly(overlay, contours, color, cv::LINE_AA);
}
