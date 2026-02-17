#include "smart_parking/ParkingPlace.h"

using namespace std::chrono_literals;

ParkingPlace::ParkingPlace(const std::array<cv::Point, 4> coords, int id)
    : _id(id), _currentState(PlaceState::INIT_STATE)
{
    // Copy 4 points
    for (int i = 0; i < 4; i++) {
        _coords[i] = coords[i];
    }
}

PlaceState ParkingPlace::getState() const{
    return _currentState;
}

bool ParkingPlace::isRecent(std::chrono::steady_clock::time_point timestamp, std::chrono::milliseconds coolDownTime){
    bool recent = timestamp - _lastEstimationTime < coolDownTime;
    return recent;
}

bool ParkingPlace::update(LightVisionData& data, std::optional<bool> info){
    if(info.has_value()){
        if(*info)_currentState = PlaceState::OCCUPIED;
        else _currentState = PlaceState::FREE;
        _lastEstimationTime = data.timestamp;
    }

    if(isRecent(data.timestamp, 500ms))return false;

    switch (_currentState)
    {
    case PlaceState::INIT_STATE:
        _lastEstimationTime = data.timestamp;
        return true;

    case PlaceState::FREE:
        if(data.hasMovement)_currentState = PlaceState::TRANSITION_IN;
        break;

    case PlaceState::OCCUPIED:
        if(data.hasMovement)_currentState = PlaceState::TRANSITION_OUT;
        break;

    default:
        if(!data.hasMovement)return true;
        break;
    }
    return false;
}

const std::array<cv::Point, 4> ParkingPlace::getCoords() const {
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
    if (_occupancyResultAsc.valid() && _occupancyResultAsc.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            _occupancyResult = _occupancyResultAsc.get();
    }

    cv::Size frameSize = frame.size();
    if(!_coordAdjust)adjustCoords(frameSize);
    struct LightVisionData data = _lightVision(frame, _coords, 0.25);
    bool needHeavyEstimation = update(data, _occupancyResult);
    if(needHeavyEstimation){
        _occupancyResultAsc = std::async(std::launch::async, 
            [this, frame]() mutable 
            {return _estimator(frame, _coords);}
        );
    }
    _occupancyResult.reset();
}