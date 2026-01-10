#include <opencv2/opencv.hpp>
#include <fstream>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <string>
#include "Parking.h"

using namespace std;
using namespace cv;
using Clock = std::chrono::high_resolution_clock;

struct AppConfig {
    bool headless = false;
    bool record = false;
    std::string outputPath = "output.mp4";
};

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

    return config;
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

int main(int argc, char** argv){
    AppConfig config = parseArgs(argc, argv);

    std::cout << "Headless: " << config.headless << std::endl;
    std::cout << "Recording: " << config.record << std::endl;

    VideoCapture cap;
    Mat frame, warped;
    Parking parking("files/coords.json", imread("files/reference.jpg"));

    VideoWriter writer;

    namedWindow("Lecture vidéo", WINDOW_NORMAL);

    int capAndWriter = getCapAndWriter(cap, writer, "files/video1.mp4", config);
    if (capAndWriter!=1) return -1;

    // FPS measurement
    auto t_prev = Clock::now();
    double fps = 0.0;
    const double alpha = 0.1; // smoothing factor
    while (true) {
        if (!cap.read(frame)) {
            cout << "Fin de la vidéo." << endl;
            break;
        }
        auto t_now = Clock::now();
        double dt = std::chrono::duration<double>(t_now - t_prev).count();
        t_prev = t_now;

        double current_fps = 1.0 / dt;
        fps = (fps == 0.0) ? current_fps : alpha * current_fps + (1.0 - alpha) * fps;
        
        //cout << "\rFPS: " << fixed << setprecision(2) << fps << flush;

        if (!parking.alignToReference(frame, warped))break;

        parking.evolve(warped);
        parking.drawParking(warped);

        Mat output;
        parking.addBanner(warped, output, fps);

        if (config.record) {
            writer.write(output);
        }

        if (!config.headless){
            imshow("Lecture vidéo", output);

            if (waitKey(1) == 27)
                break;
        }
    }

    cout << endl;
    return 0;
}
