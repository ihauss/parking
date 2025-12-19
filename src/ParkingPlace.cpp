#include "ParkingPlace.h"

const std::array<cv::Scalar, 4> COLORS = {
    cv::Scalar(0, 255, 0),  
    cv::Scalar(0, 127, 255), 
    cv::Scalar(0, 0, 255),   
    cv::Scalar(0, 255, 255)
};

ParkingPlace::ParkingPlace(const cv::Point coords[4], int id)
    : _id(id), _state(FREE)
{
    // Copier proprement les 4 points
    for (int i = 0; i < 4; i++) {
        _coords[i] = coords[i];
    }
    
    cv::Rect bbox = cv::boundingRect(std::vector<cv::Point>(_coords, _coords + 4));
   
    std::vector<cv::Point> polyLocal;
    for (int i = 0; i < 4; ++i) {
        polyLocal.emplace_back(_coords[i].x - bbox.x, _coords[i].y - bbox.y);
    }
    
    cv::Mat mask = cv::Mat::zeros(bbox.height, bbox.width, CV_8UC1);
    cv::fillPoly(_mask, std::vector<std::vector<cv::Point>>{polyLocal}, 255);
}

placeState ParkingPlace::getState() const {
    return _state;
}

const cv::Point* ParkingPlace::getCoords() const {
    return _coords;
}

void ParkingPlace::hasMovement(const cv::Mat& frame, double thresh){
    cv::Rect bbox = cv::boundingRect(std::vector<cv::Point>(_coords, _coords + 4));
    cv::Mat roi = frame(bbox);

    cv::Mat fgMask;
    _knn->apply(roi, fgMask);

    cv::bitwise_and(fgMask, mask, fgMask);

    double motionRatio = cv::countNonZero(fgMask) / (double)cv::countNonZero(mask);

    return motionRatio > thresh;
}

void ParkingPlace::changeState(const cv::Mat& frame) {
    if(hasMovement(frame, 0.05)){
        if(_state == FREE || _state == TRANSITION_IN)_state = TRANSITION_IN;
        else if(_state == OCCUPIED || _state == TRANSITION_OUT)_state = TRANSITION_OUT;
    }
    else{
        if(_state == TRANSITION_IN)_state = OCCUPIED;
        else if(_state == TRANSITION_OUT)_state = FREE;
    }
}

void ParkingPlace::drawPlace(cv::Mat& frame, cv::Mat& overlay){
    std::vector<std::vector<cv::Point>> contours;
    contours.emplace_back(_coords, _coords + 4);
    cv::Scalar color = COLORS[_state];
    int thickness = 3;
    cv::polylines(frame, contours, true, color, thickness, cv::LINE_AA);
    cv::fillPoly(overlay, contours, color, cv::LINE_AA);
}
