#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <opencv2/opencv.hpp>
#include <future>
#include "smart_parking/HeavyEstimator.h"

enum placeState {
    INIT_STATE,
    FREE,
    TRANSITION_IN,
    OCCUPIED,
    TRANSITION_OUT
};

class StateManager{
private:
    placeState _currentState;
    std::future<bool> _occupancyResult;
    HeavyEstimator _estimator;

public:
    StateManager();

    placeState getState() const;

    void operator()(bool& motion, cv::Mat& frame, cv::Point coords[4]);
};

#endif