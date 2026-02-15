#pragma once

/**
 * @file utils.h
 * @brief Utility functions for application configuration, timing and I/O.
 *
 * This module provides helper functions to:
 *  - parse command-line arguments,
 *  - compute real-time FPS,
 *  - initialize video capture and recording,
 *  - handle display and recording logic.
 */


#include <string>
#include <opencv2/opencv.hpp>
#include <filesystem>

using namespace std;
using namespace cv;
using Clock = std::chrono::high_resolution_clock;

struct AppConfig {
    bool headless = false;
    bool record = false;
    std::string outputPath = "output.mp4";
};

/**
 * @brief Parses command-line arguments and builds application configuration.
 *
 * Supported options:
 *  - --headless : disable display
 *  - --rec      : enable video recording
 *  - --output   : output video filename
 *
 * @param argc Argument count.
 * @param argv Argument values.
 * @return Parsed application configuration.
 */
AppConfig parseArgs(int argc, char** argv);

/**
 * @brief Parses command-line arguments and builds application configuration.
 *
 * Supported options:
 *  - --headless : disable display
 *  - --rec      : enable video recording
 *  - --output   : output video filename
 *
 * @param argc Argument count.
 * @param argv Argument values.
 * @return Parsed application configuration.
 */
void getFPS(double& fps, Clock::time_point & t_prev, double alpha);

/**
 * @brief Initializes video capture and optional video writer.
 *
 * Opens the input video and configures the output video writer
 * if recording is enabled.
 *
 * @param cap OpenCV video capture object.
 * @param writer OpenCV video writer object.
 * @param videoPath Path to the input video file.
 * @param config Application configuration.
 * @return Status code:
 *         - 1  : success
 *         - 0  : failed to open video
 *         - -1 : failed to read first frame
 *         - -2 : failed to open video writer
 */
int getCapAndWriter(VideoCapture& cap, VideoWriter& writer, const string& videoPath, AppConfig& config);

/**
 * @brief Handles frame recording and display.
 *
 * Writes the frame to disk if recording is enabled and displays
 * it unless running in headless mode.
 *
 * @param writer Video writer.
 * @param frame Frame to record and/or display.
 * @param config Application configuration.
 * @return False if the user requested exit, true otherwise.
 */
bool recordAndDisplay(VideoWriter& writer, Mat& frame, AppConfig& config);