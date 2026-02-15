#include "smart_parking/StateManager.h"

StateManager::StateManager()
    : _currentState(INIT_STATE)
{}

placeState StateManager::getState() const{
    return _currentState;
}

void StateManager::operator()(bool& motion, cv::Mat& frame, cv::Point coords[4]){
    /*if (_occupancyResult.valid() && _occupancyResult.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            if(_occupancyResult.get())_currentState = OCCUPIED;
            else _currentState = FREE;
    }*/

    bool occupancyResult;

    switch (_currentState)
    {
    case INIT_STATE:
        occupancyResult = _estimator(frame, coords);
        if(!occupancyResult)_currentState = FREE;
        else if(occupancyResult)_currentState = OCCUPIED;
        break;

    case FREE:
        if(motion)_currentState = TRANSITION_IN;
        break;

    case OCCUPIED:
        if(motion)_currentState = TRANSITION_OUT;
        break;

    default:
        //if(!motion)_occupancyResult = std::async(std::launch::async, [this, frame]() {return _estimator(frame);});
        std::cout << motion << std::endl;
        occupancyResult = _estimator(frame, coords);
        if(!motion && !occupancyResult)_currentState = FREE;
        else if(!motion && occupancyResult)_currentState = OCCUPIED;
        break;
    }
}