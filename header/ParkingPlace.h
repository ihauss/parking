#ifndef PARKING_PLACE_H
#define PARKING_PLACE_H

#include <opencv2/opencv.hpp>

enum placeState {
    FREE,
    TRANSITION_IN,
    OCCUPIED,
    TRANSITION_OUT
};

class ParkingPlace {
private:
    int _id;
    cv::Point _coords[4];   // Les 4 points de la place
    placeState _state;
    cv::Mat _mask;

    bool hasMovement(const cv::Mat& frame, double thresh);

public:
    // Constructeur
    ParkingPlace(const cv::Point coords[4], int id = 0);

    // Getters
    placeState getState() const;
    const cv::Point* getCoords() const;

    // Méthodes
    void changeState(const cv::Mat& frame);
    void drawPlace(cv::Mat& frame, cv::Mat& overlay);
};

#endif
