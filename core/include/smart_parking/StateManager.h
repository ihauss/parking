#pragma once

#include <opencv2/opencv.hpp>
#include <future>
#include "smart_parking/HeavyEstimator.h"
#include "smart_parking/LightVisionData.h"

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

    void operator()(LightVisionData& data, cv::Mat& frame, cv::Point coords[4]);
};