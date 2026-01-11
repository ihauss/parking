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

int main(int argc, char** argv){
    AppConfig config = parseArgs(argc, argv);

    VideoCapture cap;
    VideoWriter writer;
    Mat frame, output;
    Parking parking("files/coords.json", imread("files/reference.jpg"));

    namedWindow("Lecture vidéo", WINDOW_NORMAL);

    int capAndWriter = getCapAndWriter(cap, writer, "files/video1.mp4", config);
    if (capAndWriter!=1) return -1;

    // FPS measurement
    auto t_prev = Clock::now();
    double fps = 0.0;
    while (true) {
        if (!cap.read(frame)) {
            cout << "Fin de la vidéo." << endl;
            break;
        }

        getFPS(fps, t_prev, 0.1);

        if (!parking.alignToReference(frame, output))break;
        parking.evolve(output);
        parking.drawParking(output);
        parking.addBanner(output, output, fps);

        if(!recordAndDisplay(writer, output, config))break;
    }

    cout << endl;
    return 0;
}
