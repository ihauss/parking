#include "utils.h"

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

void getFPS(double& fps, Clock::time_point & t_prev, double alpha){
    auto t_now = Clock::now();
    double dt = std::chrono::duration<double>(t_now - t_prev).count();
    t_prev = t_now;

    double current_fps = 1.0 / dt;
    fps = (fps == 0.0) ? current_fps : alpha * current_fps + (1.0 - alpha) * fps;
}

int getCapAndWriter(VideoCapture& cap, VideoWriter& writer, const string& videoPath, AppConfig& config){
    Mat frame;
    cap.open(videoPath);
    if (!cap.isOpened()) {
        cerr << "Erreur : impossible d'ouvrir la vidéo !" << endl;
        return 0;
    }
    if(!cap.read(frame))return-1;
    if (config.record) {
        int fourcc = cv::VideoWriter::fourcc('a','v','c','1');  // ou 'H','2','6','4'

        //int fourcc = cv::VideoWriter::fourcc('m','p','4','v');
        double fpsVid = 25.0; // ou récupéré depuis la caméra
        
        cv::Size frameSize(frame.cols, frame.rows);

        string outputPath = "output/"+config.outputPath;

        namespace fs = std::filesystem;

        const std::string outputDir = "output";

        if (!fs::exists(outputDir)) {
            fs::create_directories(outputDir);
            std::cout << "Created output directory: " << outputDir << std::endl;
        }

        writer.open(outputPath, fourcc, fpsVid, frameSize);

        if (!writer.isOpened()) {
            std::cerr << "Failed to open video writer" << std::endl;
            return -2;
        }
    }
    return 1;
}

bool recordAndDisplay(VideoWriter& writer, Mat& frame, AppConfig& config){
    if (config.record) {
        writer.write(frame);
    }

    if (!config.headless){
        imshow("Lecture vidéo", frame);

        if (waitKey(1) == 27)
            return false;
    }
    return true;
}