#ifndef PARKING_H
#define PARKING_H

#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <fstream>
#include "ParkingPlace.h"

class Parking {
private:
    int _numOccupied;
    std::vector<ParkingPlace> _places;
    cv::Mat _reference;
    
    cv::Mat _prevGray;
    std::vector<cv::Point2f> _prevPts;
    std::vector<cv::Point2f> _refPts;
    bool _flowInitialized = false;

    void initReference();


public:
    Parking(const std::string& jsonPath, cv::Mat reference);

    size_t getNumPlace() const;
    int getNumOccupied() const;

    void evolve(const cv::Mat& frame);
    void drawParking(cv::Mat& frame);
    bool alignToReference(const cv::Mat& frame, cv::Mat& warped);
    void addBanner(const cv::Mat& frame, cv::Mat& output, const double& fps);
};

#endif
