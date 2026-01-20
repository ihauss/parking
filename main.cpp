/**
 * @file main.cpp
 * @brief Entry point of the parking occupancy detection application.
 *
 * This application processes a video stream to estimate parking slot
 * occupancy, annotate the frames with visual information, and optionally
 * display or record the processed video depending on the user configuration.
 */


#include <opencv2/opencv.hpp>
#include <fstream>
#include <chrono>
#include <iostream>
#include <string>
#include "Parking.h"
#include "utils.h"

using namespace std;
using namespace cv;
using Clock = std::chrono::high_resolution_clock;

/**
 * @brief Application entry point.
 *
 * Initializes the application configuration, video input/output streams,
 * and core processing components, then runs the main frame-by-frame
 * processing loop.
 *
 * @param argc Number of command-line arguments.
 * @param argv Command-line arguments.
 * @return Exit status code.
 */
int main(int argc, char** argv){
    // Parse command-line arguments (recording mode, headless mode, etc.)
    AppConfig config = parseArgs(argc, argv);

    VideoCapture cap;
    VideoWriter writer;
    Mat frame, output;

    // Initialize the parking detection system
    Parking parking("files/coords.json", imread("files/reference.jpg"));

    // Create display window
    namedWindow("Parking view", WINDOW_NORMAL);

    // Initialize video capture and optional recording
    int capAndWriter = getCapAndWriter(cap, writer, "files/video1.mp4", config);
    if (capAndWriter!=1) return -1;

    auto t_prev = Clock::now();
    double fps = 0.0;
    while (true) {
        // Read next frame from the video stream
        if (!cap.read(frame)) {
            cout << "Video end" << endl;
            break;
        }

        // Update FPS estimation
        getFPS(fps, t_prev, 0.1);

        // Align current frame with the reference image
        if (!parking.alignToReference(frame, output))break;

        // Update parking occupancy state
        parking.evolve(output);

        // Draw parking slots and occupancy status
        parking.drawParking(output);

        // Add informational overlay (FPS, statistics, etc.)
        parking.addBanner(output, output, fps);

        // Display and/or record the processed frame
        if(!recordAndDisplay(writer, output, config))break;
    }

    cout << endl;
    return 0;
}
