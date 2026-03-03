#pragma once

/**
 * @file utils.h
 * @brief Application-level utility helpers.
 *
 * This module contains helper structures and functions intended for the
 * application entry point (`main`) and runtime orchestration.
 *
 * Responsibilities covered here are deliberately non-domain-specific:
 *  - parsing command-line arguments,
 *  - initializing video input/output,
 *  - handling frame display and recording.
 *
 * This file is considered part of the execution layer, not the core
 * parking domain logic. It may be replaced or adapted in the future
 * for embedded targets (Raspberry Pi) or ROS2-based execution.
 */

#include <string>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

/**
 * @brief Status codes returned during video initialization.
 *
 * Used to explicitly describe failure modes when initializing
 * video capture and recording.
 */
enum class InitStatus {
    /// Initialization completed successfully
    Success,

    /// Failed to open input video
    VideoOpenFailed,

    /// Failed to read the first frame from input video
    FirstFrameReadFailed,

    /// Failed to open output video writer
    WriterOpenFailed
};

/**
 * @brief Application runtime configuration.
 *
 * This structure aggregates all runtime flags derived from
 * command-line arguments and is intended to be immutable
 * after initialization.
 */
struct AppConfig {
    /**
     * @brief Disable all GUI display if true.
     *
     * Useful for headless execution on embedded systems
     * or server-side processing.
     */
    bool headless = false;

    /**
     * @brief Enable video recording if true.
     */
    bool record = false;

    /**
     * @brief Output path for recorded video.
     *
     * Only used when recording is enabled.
     */
    std::string outputPath = "output.avi";
};

/**
 * @brief Parses command-line arguments and builds application configuration.
 *
 * Supported options:
 *  - --headless : disable frame display
 *  - --rec      : enable video recording
 *  - --output   : output video filename
 *
 * This function is intended to be used exclusively by the
 * application entry point.
 *
 * @param argc Argument count.
 * @param argv Argument values.
 * @return Parsed application configuration.
 */
AppConfig parseArgs(int argc, char** argv);

/**
 * @brief Initializes video capture and optional video writer.
 *
 * Opens the input video file and validates that at least one frame
 * can be read. If recording is enabled, initializes the video writer
 * using the input video properties.
 *
 * @param cap OpenCV video capture object to initialize.
 * @param writer OpenCV video writer object to initialize if recording is enabled.
 * @param videoPath Path to the input video file.
 * @param config Application configuration.
 *
 * @return Initialization status describing success or failure cause.
 */
InitStatus getCapAndWriter(
    cv::VideoCapture& cap,
    cv::VideoWriter& writer,
    const std::string& videoPath,
    const AppConfig& config
);

/**
 * @brief Handles frame display and optional recording.
 *
 * Writes the frame to disk if recording is enabled and displays it
 * unless running in headless mode.
 *
 * This function also handles user exit requests (e.g. key press).
 *
 * @param writer Video writer.
 * @param frame Frame to display and/or record.
 * @param config Application configuration.
 *
 * @return False if the user requested application exit, true otherwise.
 */
bool recordAndDisplay(
    cv::Mat& frame,
    const AppConfig& config
);

void stopRecorder();