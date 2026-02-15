#pragma once

#include <opencv2/opencv.hpp>
#include <future>
#include <chrono>
#include "smart_parking/HeavyEstimator.h"
#include "smart_parking/LightVisionData.h"

using namespace std::chrono_literals;

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
    std::chrono::steady_clock::time_point _lastEstimationTime;

public:
    StateManager();

    placeState getState() const;

    bool isRecent(std::chrono::steady_clock::time_point timestamp, std::chrono::milliseconds coolDownTime);

    void operator()(LightVisionData& data, cv::Mat& frame, cv::Point coords[4]);
};