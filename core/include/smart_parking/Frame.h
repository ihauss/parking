#pragma once

#include <opencv2/opencv.hpp>
#include <chrono>

namespace smart_parking {

/**
 * @enum PixelFormat
 * @brief Supported pixel formats for Frame.
 */
enum class PixelFormat {
    BGR_UINT8
};

/**
 * @struct Frame
 * @brief Lightweight image container used throughout the pipeline.
 *
 * This struct wraps a cv::Mat along with metadata such as timestamp
 * and pixel format.
 *
 * Important:
 * - cv::Mat is shallow-copied by default (shared memory)
 * - No deep copy is performed unless explicitly requested
 *
 * Responsibilities:
 *  - Transport image data between modules
 *  - Provide basic validation
 *
 * Non-responsibilities:
 *  - Does NOT own exclusive image memory
 *  - Does NOT enforce immutability
 */
struct Frame {
    cv::Mat data;   ///< Image data (BGR, uint8, HWC)
    PixelFormat format{PixelFormat::BGR_UINT8};
    std::chrono::steady_clock::time_point timestamp;

    /**
     * @brief Construct a frame from an OpenCV matrix.
     *
     * @param mat Input image (shared, not copied)
     * @param ts Optional timestamp (default = now)
     */
    explicit Frame(const cv::Mat& mat,
                   std::chrono::steady_clock::time_point ts =
                       std::chrono::steady_clock::now())
        : data(mat), timestamp(ts)
    {}

    /**
     * @brief Checks whether the frame is valid for processing.
     *
     * @return true if valid, false otherwise
     */
    bool isValid() const noexcept {
        if (data.empty())
            return false;

        if (format != PixelFormat::BGR_UINT8)
            return false;

        return data.type() == CV_8UC3 && data.isContinuous();
    }
};

} // namespace smart_parking