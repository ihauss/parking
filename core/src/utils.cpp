#include "smart_parking/utils.h"

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

InitStatus getCapAndWriter(cv::VideoCapture& cap,
                    cv::VideoWriter& writer,
                    const std::string& videoPath,
                    const AppConfig& config)
{
    cv::Mat frame;

    // Open input video
    cap.open(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "Error : impossible to open video !" << std::endl;
        return InitStatus::VideoOpenFailed;
    }

    // Read first frame to get video properties
    if (!cap.read(frame))
        return InitStatus::FirstFrameReadFailed;

    // Initialize video writer if recording is enabled
    if (config.record) {
        int fourcc = cv::VideoWriter::fourcc('a','v','c','1');
        double fpsVid = 25.0;

        cv::Size frameSize(frame.cols, frame.rows);
        std::string outputPath = "output/" + config.outputPath;

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
            return InitStatus::WriterOpenFailed;
        }
    }

    return InitStatus::Success;
}

bool recordAndDisplay(cv::VideoWriter& writer,
                      cv::Mat& frame,
                      const AppConfig& config)
{
    // Write frame to disk if recording
    if (config.record) {
        writer.write(frame);
    }

    // Display frame unless running headless
    if (!config.headless) {
        cv::imshow("Parking view", frame);

        // Exit on ESC key
        if (cv::waitKey(1) == 27)
            return false;
    }

    return true;
}
