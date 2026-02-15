#ifndef PARKING_H
#define PARKING_H

#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <fstream>
#include "ParkingPlace.h"

/**
 * @class Parking
 * @brief Manages parking occupancy detection and visualization.
 *
 * The Parking class loads parking slot definitions from a JSON file,
 * tracks frame-to-frame motion to align video frames to a reference image,
 * updates the occupancy state of each parking place, and provides
 * visualization utilities for rendering parking status and statistics.
 */
class Parking {
private:
    int _numOccupied;
    std::vector<ParkingPlace> _places;
    cv::Mat _reference;
    
    cv::Mat _prevGray;
    std::vector<cv::Point2f> _prevPts;
    std::vector<cv::Point2f> _refPts;
    bool _flowInitialized = false;

    /**
     * @brief Initializes reference features for optical flow tracking.
     *
     * Detects stable feature points in the reference image that will be
     * used to align incoming frames using optical flow.
     */
    void initReference();


public:
    /**
     * @brief Constructs a Parking object from a configuration file and reference image.
     *
     * Loads parking place definitions from a JSON file, initializes internal
     * data structures, and prepares the reference frame used for motion alignment.
     *
     * @param jsonPath Path to the JSON file describing parking places.
     * @param reference Reference image used for frame alignment.
     */
    Parking(const std::string& jsonPath, cv::Mat reference);


    /**
     * @brief Returns the total number of parking places.
     *
     * @return Number of parking places.
     */
    size_t getNumPlace() const;

    /**
     * @brief Returns the number of currently occupied parking places.
     *
     * @return Number of occupied places.
     */
    int getNumOccupied() const;

    /**
     * @brief Updates the occupancy state of all parking places.
     *
     * Evaluates each parking place using the current frame and updates
     * the global occupancy count accordingly.
     *
     * @param frame Current video frame.
     */
    void evolve(cv::Mat& frame);

    /**
     * @brief Updates the occupancy state of all parking places.
     *
     * Evaluates each parking place using the current frame and updates
     * the global occupancy count accordingly.
     *
     * @param frame Current video frame.
     */
    void drawParking(cv::Mat& frame);

    /**
     * @brief Aligns the current frame to the reference image.
     *
     * Uses optical flow and affine transformation estimation to compensate
     * for camera motion and stabilize the frame.
     *
     * @param frame Input frame to be aligned.
     * @param warped Output aligned frame.
     * @return True if alignment succeeds, false otherwise.
     */
    bool alignToReference(const cv::Mat& frame, cv::Mat& warped);

    /**
     * @brief Adds an informational banner to the output frame.
     *
     * Displays runtime information such as FPS and parking occupancy
     * statistics on a banner overlay.
     *
     * @param frame Original frame.
     * @param output Frame with banner added.
     * @param fps Current frames per second.
     */
    void addBanner(const cv::Mat& frame, cv::Mat& output, const double& fps);
};

#endif
