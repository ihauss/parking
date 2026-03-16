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

#include "smart_parking/Parking.h"
#include "smart_parking/Frame.h"
#include "smart_parking/RenderSnapshot.h"
#include "Renderer.h"
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
    Frame engineFrame(frame);
    Renderer renderer;

    // Initialize the parking detection system
    Parking parking("files/coords.json", imread("files/reference.jpg"));

    // Create display window
    if (!config.headless)namedWindow("Parking view", WINDOW_NORMAL);

    // Initialize video capture and optional recording
    InitStatus capAndWriter = getCapAndWriter(cap, writer, "files/video1.mp4", config);
    if (capAndWriter!=InitStatus::Success) return -1;

    while (true) {
        // Read next frame from the video stream
        if (!cap.read(frame)) {
            cout << "Video end" << endl;
            break;
        }

        // Update parking occupancy state
        engineFrame.data = frame;
        engineFrame.timestamp = std::chrono::steady_clock::now();
        parking.evolve(engineFrame);

        RenderSnapshot snapshot = parking.getRenderSnapshot();

        renderer(frame, output, snapshot);
        
        // Display and/or record the processed frame
        if(!recordAndDisplay(output, config))break;
    }

    stopRecorder();

    cout << endl;
    return 0;
}
