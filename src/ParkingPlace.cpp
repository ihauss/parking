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
    // Copy 4 points
    for (int i = 0; i < 4; i++) {
        _coords[i] = coords[i];
    }

    initMask();
    _knn = cv::createBackgroundSubtractorKNN(_warmUp);
}

placeState ParkingPlace::evalState(const cv::Mat& frame){
    float W = cv::norm(_coords[0] - _coords[1]);
    float H = cv::norm(_coords[0] - _coords[3]);

    std::vector<cv::Point2f> srcPts = {
        cv::Point2f(_coords[0]),
        cv::Point2f(_coords[1]),
        cv::Point2f(_coords[2]),
        cv::Point2f(_coords[3])
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

    cv::Mat ycrcb;
    cv::cvtColor(warped, ycrcb, cv::COLOR_BGR2YCrCb);

    std::vector<cv::Mat> channels;
    cv::split(ycrcb, channels);

    cv::Mat Y = channels[0];

    // variance
    cv::Scalar mean, stddev;
    cv::meanStdDev(Y, mean, stddev);
    double varianceY = stddev[0] * stddev[0];
    
    _warmUp = 10;
    if(varianceY > 1500)return OCCUPIED;

    return FREE;
}

void ParkingPlace::initMask(){
    cv::Rect bbox = cv::boundingRect(std::vector<cv::Point>(_coords, _coords + 4));
   
    std::vector<cv::Point> polyLocal;
    for (int i = 0; i < 4; ++i) {
        polyLocal.emplace_back(_coords[i].x - bbox.x, _coords[i].y - bbox.y);
    }
    
    _mask = cv::Mat::zeros(bbox.height, bbox.width, CV_8UC1);
    cv::fillPoly(_mask, std::vector<std::vector<cv::Point>>{polyLocal}, 255);
}

placeState ParkingPlace::getState() const {
    return _state;
}

const cv::Point* ParkingPlace::getCoords() const {
    return _coords;
}

bool ParkingPlace::adjustCoords(const cv::Size& frameSize)
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

    initMask();
    _coordAdjust = true;

    return modified;
}

bool ParkingPlace::hasMovement(const cv::Mat& frame, double thresh){
    cv::Rect bbox = cv::boundingRect(std::vector<cv::Point>(_coords, _coords + 4));
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

void ParkingPlace::changeState(const cv::Mat& frame) {

    if(!_coordAdjust){
        adjustCoords(frame.size());
        _futureState = std::async(std::launch::async, [this, frame]() {return evalState(frame);});
    }
    if (_futureState.valid() && _futureState.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            _state = _futureState.get();
    }
    if(hasMovement(frame, 0.25)){
        if(_state == FREE || _state == TRANSITION_IN)_state = TRANSITION_IN;
        else if(_state == OCCUPIED || _state == TRANSITION_OUT)_state = TRANSITION_OUT;
    }
    else{
        if(_state == TRANSITION_IN)_futureState = std::async(std::launch::async, [this, frame]() {return evalState(frame);});
        else if(_state == TRANSITION_OUT)_futureState = std::async(std::launch::async, [this, frame]() {return evalState(frame);});
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
