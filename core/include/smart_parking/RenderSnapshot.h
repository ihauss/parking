#pragma once

#include <vector>
#include <array>
#include "RenderPlace.h"

struct RenderSnapshot {
    std::vector<RenderPlace> places;

    bool hasAffine{false};
    std::array<double, 6> affine{};

    int numOccupied{0};
    int numPlaces{0};
};