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

AppConfig parseArgs(int argc, char** argv);

void getFPS(double& fps, Clock::time_point & t_prev, double alpha);

int getCapAndWriter(VideoCapture& cap, VideoWriter& writer, const string& videoPath, AppConfig& config);

bool recordAndDisplay(VideoWriter& writer, Mat& frame, AppConfig& config);