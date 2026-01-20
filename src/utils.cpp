#include "utils.h"

AppConfig parseArgs(int argc, char** argv) {
    AppConfig config;

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);

        if (arg == "--headless") {
            config.headless = true;   // Disable display
        }
        else if (arg == "--rec") {
            config.record = true;     // Enable video recording
        }
        else if (arg == "--output" && i + 1 < argc) {
            config.outputPath = argv[++i]; // Output video filename
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
        }
    }

    // Debug / runtime configuration summary
    std::cout << "Headless: " << config.headless << std::endl;
    std::cout << "Recording: " << config.record << std::endl;

    return config;
}

void getFPS(double& fps, Clock::time_point& t_prev, double alpha){
    // Measure elapsed time since last frame
    auto t_now = Clock::now();
    double dt = std::chrono::duration<double>(t_now - t_prev).count();
    t_prev = t_now;

    // Instantaneous FPS
    double current_fps = 1.0 / dt;

    // Exponential moving average for smoother FPS
    fps = (fps == 0.0)
        ? current_fps
        : alpha * current_fps + (1.0 - alpha) * fps;
}

int getCapAndWriter(VideoCapture& cap,
                    VideoWriter& writer,
                    const string& videoPath,
                    AppConfig& config)
{
    Mat frame;

    // Open input video
    cap.open(videoPath);
    if (!cap.isOpened()) {
        cerr << "Error : impossible to open video !" << endl;
        return 0;
    }

    // Read first frame to get video properties
    if (!cap.read(frame))
        return -1;

    // Initialize video writer if recording is enabled
    if (config.record) {
        int fourcc = cv::VideoWriter::fourcc('a','v','c','1');
        double fpsVid = 25.0;

        cv::Size frameSize(frame.cols, frame.rows);
        string outputPath = "output/" + config.outputPath;

        namespace fs = std::filesystem;
        const std::string outputDir = "output";

        // Create output directory if needed
        if (!fs::exists(outputDir)) {
            fs::create_directories(outputDir);
            std::cout << "Created output directory: "
                      << outputDir << std::endl;
        }

        writer.open(outputPath, fourcc, fpsVid, frameSize);

        if (!writer.isOpened()) {
            std::cerr << "Failed to open video writer" << std::endl;
            return -2;
        }
    }

    return 1;
}

bool recordAndDisplay(VideoWriter& writer,
                      Mat& frame,
                      AppConfig& config)
{
    // Write frame to disk if recording
    if (config.record) {
        writer.write(frame);
    }

    // Display frame unless running headless
    if (!config.headless) {
        imshow("Parking view", frame);

        // Exit on ESC key
        if (waitKey(1) == 27)
            return false;
    }

    return true;
}
