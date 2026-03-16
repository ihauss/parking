#pragma once

#include <vector>
#include <array>
#include "smart_parking/RenderPlace.h"

struct RenderSnapshot {
    std::vector<RenderPlace> places;

    bool hasAffine{false};
    std::array<double, 6> affine{};

    int numOccupied{0};
    int numPlaces{0};
};