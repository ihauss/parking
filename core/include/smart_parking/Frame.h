#pragma once

#include <opencv2/opencv.hpp>
#include <chrono>

enum class PixelFormat {
    BGR_UINT8
};

struct Frame {
    cv::Mat data;   // Must be BGR, uint8, HWC, contiguous
    PixelFormat format{PixelFormat::BGR_UINT8};
    std::chrono::steady_clock::time_point timestamp;

    bool isValid() const noexcept {
        return !data.empty()
               && data.type() == CV_8UC3
               && data.isContinuous();
    }
};