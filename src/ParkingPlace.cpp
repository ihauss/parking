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
}

placeState ParkingPlace::getState() const {
    return _state;
}

const cv::Point* ParkingPlace::getCoords() const {
    return _coords;
}

void ParkingPlace::changeState(const cv::Mat& frame) {
    // TODO : analyser frame dans le polygone défini par coords[4]
}

void ParkingPlace::drawPlace(cv::Mat& frame, cv::Mat& overlay){
    std::vector<std::vector<cv::Point>> contours;
    contours.emplace_back(_coords, _coords + 4);
    cv::Scalar color = COLORS[_state];
    int thickness = 3;
    cv::polylines(frame, contours, true, color, thickness, cv::LINE_AA);
    cv::fillPoly(overlay, contours, color, cv::LINE_AA);
}
