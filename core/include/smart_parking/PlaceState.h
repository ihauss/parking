#pragma once

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
 *      Used at startup before any reliable observation is available.
 *
 *  - FREE:
 *      No vehicle is present.
 *      No significant motion detected inside the parking area.
 *
 *  - TRANSITION_IN:
 *      Motion has been detected and a vehicle is likely entering.
 *      The final occupancy is not yet confirmed.
 *      A heavy estimation may be running asynchronously.
 *
 *  - OCCUPIED:
 *      The parking place is confirmed occupied.
 *      No significant motion is expected inside the slot.
 *
 *  - TRANSITION_OUT:
 *      Motion suggests that a vehicle is leaving the slot.
 *      The final free state is not yet confirmed.
 */
enum class PlaceState {
    INIT_STATE,
    FREE,
    TRANSITION_IN,
    OCCUPIED,
    TRANSITION_OUT
};
