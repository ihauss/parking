#pragma once

namespace smart_parking {

/**
 * @enum PlaceState
 * @brief Logical state of a parking place.
 *
 * This enum defines the finite state machine used to track
 * the occupancy of a parking slot over time.
 *
 * State semantics:
 *
 *  - INIT_STATE:
 *      Initial undefined state.
 *
 *  - FREE:
 *      No vehicle is present.
 *
 *  - TRANSITION_IN:
 *      Vehicle likely entering.
 *
 *  - OCCUPIED:
 *      Parking place is occupied.
 *
 *  - TRANSITION_OUT:
 *      Vehicle likely leaving.
 *
 * ⚠️ Note:
 * The numeric values are stable and should not be changed
 * if used in serialization or bindings.
 */
enum class PlaceState {
    INIT_STATE = 0,
    FREE = 1,
    TRANSITION_IN = 2,
    OCCUPIED = 3,
    TRANSITION_OUT = 4
};

/**
 * @brief Convert PlaceState to string (for logging/debugging)
 */
inline const char* to_string(PlaceState state) {
    switch (state) {
        case PlaceState::INIT_STATE: return "INIT_STATE";
        case PlaceState::FREE: return "FREE";
        case PlaceState::TRANSITION_IN: return "TRANSITION_IN";
        case PlaceState::OCCUPIED: return "OCCUPIED";
        case PlaceState::TRANSITION_OUT: return "TRANSITION_OUT";
    }
    return "UNKNOWN";
}

/**
 * @brief Returns true if state is stable (no transition ongoing)
 */
inline bool isStable(PlaceState state) {
    return state == PlaceState::FREE || state == PlaceState::OCCUPIED;
}

/**
 * @brief Returns true if state is a transition state
 */
inline bool isTransition(PlaceState state) {
    return state == PlaceState::TRANSITION_IN ||
           state == PlaceState::TRANSITION_OUT;
}

} // namespace smart_parking