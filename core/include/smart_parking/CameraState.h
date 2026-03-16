#pragma once

#include <string>

enum class CameraState {
    IDLE,
    RUNNING,
    ERROR
};

inline std::string to_string(CameraState state) {
    switch (state) {
        case CameraState::IDLE:
            return "IDLE";
        case CameraState::RUNNING:
            return "RUNNING";
        case CameraState::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}