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

    Frame(const cv::Mat& mat)
        : data(mat),
        timestamp(std::chrono::steady_clock::now())
    {}

    bool isValid() const noexcept {
        if (data.empty())
            return false;

        if (format != PixelFormat::BGR_UINT8)
            return false;

        return data.type() == CV_8UC3 && data.isContinuous();
    }
};