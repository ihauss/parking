#pragma once

#include <vector>
#include <array>

#include "smart_parking/RenderPlace.h"

namespace smart_parking {

/**
 * @struct RenderSnapshot
 * @brief Snapshot of the parking system state for rendering or API exposure.
 *
 * This structure aggregates all rendering-relevant information at a given time.
 * It is designed to be lightweight and easily transferable (e.g. Python bindings).
 *
 * Invariants:
 *  - numPlaces == places.size()
 *  - numOccupied must match the number of OCCUPIED places
 *  - affine is valid only if hasAffine == true
 */
struct RenderSnapshot {

    /// List of parking places (render descriptors)
    std::vector<RenderPlace> places;

    /// Whether a valid affine transform is available
    bool hasAffine{false};

    /// Affine transform in row-major order:
    /// [a, b, c, d, e, f] such that:
    /// x' = ax + by + c
    /// y' = dx + ey + f
    std::array<double, 6> affine{};

    /// Number of occupied places (cached for performance)
    int numOccupied{0};

    /// Total number of places (should match places.size())
    int numPlaces{0};
};

} // namespace smart_parking