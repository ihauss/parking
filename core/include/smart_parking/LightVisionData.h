#pragma once

#include <chrono>

struct LightVisionData
{
    bool hasMovement;
    std::chrono::steady_clock::time_point timestamp;

};