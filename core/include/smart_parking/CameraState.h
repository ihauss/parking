#pragma once

#include <string>

namespace smart_parking {

/**
 * @enum CameraState
 * @brief Represents the runtime state of a camera in the system.
 *
 * States:
 *  - IDLE: Camera is initialized but not processing frames
 *  - RUNNING: Camera is actively processing frames
 *  - ERROR: Camera encountered a failure
 */
enum class CameraState {
    IDLE,
    RUNNING,
    ERROR
};

/**
 * @brief Converts CameraState to a human-readable string.
 *
 * Intended for logging and debugging purposes.
 *
 * @param state Camera state
 * @return const char* string representation
 */
inline const char* to_string(CameraState state) {
    switch (state) {
        case CameraState::IDLE: return "IDLE";
        case CameraState::RUNNING: return "RUNNING";
        case CameraState::ERROR: return "ERROR";
    }
    return "UNKNOWN";
}

} // namespace smart_parking