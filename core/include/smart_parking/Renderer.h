#pragma once

#include <opencv2/opencv.hpp>
#include "smart_parking/RenderPlace.h"

using Clock = std::chrono::high_resolution_clock;

/**
 * @class Renderer
 * @brief Visualization utility for parking place monitoring.
 *
 * This class is responsible for:
 *  - Drawing parking places overlays
 *  - Displaying occupancy states
 *  - Computing and displaying FPS
 *  - Rendering informational banners
 *
 * It does NOT:
 *  - Modify parking states
 *  - Perform detection or decision logic
 */
class Renderer {
private:
    /// Smoothed frames-per-second value
    double _fps = 0.0;

    /// Timestamp of previous frame
    Clock::time_point _tPrev = Clock::now();

public:
    /**
     * @brief Updates the FPS estimation.
     *
     * Applies exponential smoothing to stabilize FPS display.
     *
     * @param alpha Smoothing factor (0 < alpha <= 1).
     */
    void updateFPS(double alpha);

    /**
     * @brief Draws parking places on a frame.
     *
     * Renders each parking place using its geometry and state.
     *
     * @param frame Frame to draw onto (modified in-place).
     * @param places Parking places rendering descriptors.
     */
    void draw(cv::Mat& frame, const std::vector<RenderPlace>& places);

    /**
     * @brief Adds an informational banner to the frame.
     *
     * Displays runtime information such as FPS and occupancy statistics.
     *
     * @param frame Input frame.
     * @param output Output frame with banner.
     * @param numOccupied Number of occupied places.
     * @param numPlace Total number of places.
     */
    void addBanner(
        const cv::Mat& frame,
        cv::Mat& output,
        int numOccupied,
        int numPlace
    );

    /**
     * @brief Full rendering pipeline.
     *
     * Combines drawing, banner rendering, and FPS update into a single call.
     *
     * @param frame Input frame.
     * @param places Parking places to render.
     * @param output Final rendered frame.
     * @param numOccupied Number of occupied places.
     * @param numPlace Total number of places.
     */
    void operator()(
        cv::Mat& frame,
        const std::vector<RenderPlace>& places,
        cv::Mat& output,
        int numOccupied,
        int numPlace
    );
};
