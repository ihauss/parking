#include "smart_parking/StateManager.h"

StateManager::StateManager()
    : _currentState(INIT_STATE)
{}

placeState StateManager::getState() const{
    return _currentState;
}

void StateManager::operator()(LightVisionData& data, cv::Mat& frame, cv::Point coords[4]){
    bool motion = data.hasMovement;
    if (_occupancyResult.valid() && _occupancyResult.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            if(_occupancyResult.get())_currentState = OCCUPIED;
            else _currentState = FREE;
    }

    switch (_currentState)
    {
    case INIT_STATE:
        _occupancyResult = std::async(std::launch::async, [this, frame, coords]() mutable {return _estimator(frame, coords);});
        break;

    case FREE:
        if(motion)_currentState = TRANSITION_IN;
        break;

    case OCCUPIED:
        if(motion)_currentState = TRANSITION_OUT;
        break;

    default:
        if(!motion)_occupancyResult = std::async(std::launch::async, [this, frame, coords]() mutable {return _estimator(frame, coords);});
        
        break;
    }
}