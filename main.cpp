#include <opencv2/opencv.hpp>
#include <fstream>
#include <chrono>
#include <iostream>
#include "Parking.h"

using namespace std;
using namespace cv;
using Clock = std::chrono::high_resolution_clock;

int getCap(VideoCapture& cap, const string& videoPath){
    cap.open(videoPath);
    if (!cap.isOpened()) {
        cerr << "Erreur : impossible d'ouvrir la vidéo !" << endl;
        return 0;
    }
    return 1;
}

int main(){
    VideoCapture cap;
    Mat frame, warped;
    Parking parking("files/coords.json", imread("files/reference.jpg"));

    namedWindow("Lecture vidéo", WINDOW_NORMAL);

    if (!getCap(cap, "files/video1.mp4")) return -1;

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

        imshow("Lecture vidéo", output);

        if (waitKey(1) == 27)
            break;
    }

    cout << endl;
    return 0;
}
