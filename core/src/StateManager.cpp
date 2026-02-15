#include "smart_parking/StateManager.h"

StateManager::StateManager()
    : _currentState(INIT_STATE)
{}

placeState StateManager::getState() const{
    return _currentState;
}

bool StateManager::isRecent(std::chrono::steady_clock::time_point timestamp, std::chrono::milliseconds coolDownTime){
    bool recent = timestamp - _lastEstimationTime < coolDownTime;
    _lastEstimationTime = timestamp;
    return recent;
}

void StateManager::operator()(LightVisionData& data, cv::Mat& frame, cv::Point coords[4]){
    if (_occupancyResult.valid() && _occupancyResult.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            if(_occupancyResult.get())_currentState = OCCUPIED;
            else _currentState = FREE;
    }

    switch (_currentState)
    {
    case INIT_STATE:
        _occupancyResult = std::async(std::launch::async, [this, frame, coords]() mutable {return _estimator(frame, coords);});
        _lastEstimationTime = data.timestamp;
        break;

    case FREE:
        if(data.hasMovement)_currentState = TRANSITION_IN;
        break;

    case OCCUPIED:
        if(data.hasMovement)_currentState = TRANSITION_OUT;
        break;

    default:
        if(isRecent(data.timestamp, 50ms))return;
        if(!data.hasMovement)_occupancyResult = std::async(std::launch::async, [this, frame, coords]() mutable {return _estimator(frame, coords);});
        
        break;
    }
}