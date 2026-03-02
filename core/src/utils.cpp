#include "smart_parking/utils.h"

// ===============================
// Async Recorder Globals
// ===============================

static std::queue<cv::Mat> g_frameQueue;
static std::mutex g_queueMutex;
static std::condition_variable g_condition;
static std::atomic<bool> g_recordingActive(false);
static std::thread g_writerThread;

// ===============================
// Recorder Thread Function
// ===============================

static void writerWorker(cv::VideoWriter* writer)
{
    while (g_recordingActive || !g_frameQueue.empty()) {

        std::unique_lock<std::mutex> lock(g_queueMutex);
        g_condition.wait(lock, [] {
            return !g_recordingActive || !g_frameQueue.empty();
        });

        while (!g_frameQueue.empty()) {
            cv::Mat frame = g_frameQueue.front();
            g_frameQueue.pop();
            lock.unlock();

            writer->write(frame);

            lock.lock();
        }
    }
}

// ===============================
// Argument Parsing
// ===============================

AppConfig parseArgs(int argc, char** argv) {
    AppConfig config;

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);

        if (arg == "--headless") {
            config.headless = true;
        }
        else if (arg == "--rec") {
            config.record = true;
        }
        else if (arg == "--output" && i + 1 < argc) {
            config.outputPath = argv[++i];
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
        }
    }

    std::cout << "Headless: " << config.headless << std::endl;
    std::cout << "Recording: " << config.record << std::endl;

    return config;
}

// ===============================
// Capture + Writer Init
// ===============================

InitStatus getCapAndWriter(cv::VideoCapture& cap,
                           cv::VideoWriter& writer,
                           const std::string& videoPath,
                           const AppConfig& config)
{
    cv::Mat frame;

    cap.open(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "Error : impossible to open video !" << std::endl;
        return InitStatus::VideoOpenFailed;
    }

    if (!cap.read(frame))
        return InitStatus::FirstFrameReadFailed;

    if (config.record) {

        int fourcc = cv::VideoWriter::fourcc('M','J','P','G'); // lighter codec
        double fpsVid = 14.0; // align with real FPS

        cv::Size frameSize(frame.cols, frame.rows);
        std::string outputPath = "output/" + config.outputPath;

        namespace fs = std::filesystem;
        const std::string outputDir = "output";

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

        // Start async writer
        g_recordingActive = true;
        g_writerThread = std::thread(writerWorker, &writer);
    }

    return InitStatus::Success;
}

// ===============================
// Record + Display
// ===============================

bool recordAndDisplay(cv::Mat& frame,
                      const AppConfig& config)
{
    if (config.record) {
        {
            std::lock_guard<std::mutex> lock(g_queueMutex);
            g_frameQueue.push(frame.clone());
        }
        g_condition.notify_one();
    }

    if (!config.headless) {
        cv::imshow("Parking view", frame);
        if (cv::waitKey(1) == 27){
            return false;
        }
    }

    return true;
}

// ===============================
// Cleanup function (call before exit)
// ===============================

void stopRecorder(){
    g_recordingActive = false;
    g_condition.notify_all();

    if (g_writerThread.joinable())
        g_writerThread.join();
}