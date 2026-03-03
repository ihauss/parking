#pragma once

#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <stdexcept>

#include "ParkingPlace.h"
#include "smart_parking/Aligner.h"
#include "smart_parking/RenderPlace.h"
#include "smart_parking/Logger.h"
#include "Frame.h"
#include "RenderSnapshot.h"

/**
 * @class Parking
 * @brief High-level manager for parking occupancy estimation and rendering.
 *
 * The Parking class represents a full parking area composed of multiple
 * ParkingPlace instances. It is responsible for:
 *
 *  - loading parking place definitions from a configuration file,
 *  - aligning incoming frames to a reference image,
 *  - updating the occupancy state of each parking place,
 *  - aggregating global statistics (e.g. number of occupied places),
 *  - providing rendering-friendly data for visualization layers.
 *
 * This class does NOT perform low-level vision algorithms itself; it
 * orchestrates specialized components (Aligner, ParkingPlace, Renderer).
 */
class Parking {
private:
    /**
     * @brief Number of currently occupied parking places.
     *
     * This value is recomputed at each evolve() call based on the
     * state of all ParkingPlace instances.
     */
    int _numOccupied{0};

    /**
     * @brief Collection of managed parking places.
     */
    std::vector<ParkingPlace> _places;

    /**
     * @brief Last observed latency of the full pipeline.
     */
    double _lastLatencyMs{0.0};

    /**
     * @brief Frame-to-frame alignment helper.
     *
     * Used to compensate for camera motion by aligning incoming frames
     * to a fixed reference image.
     */
    Aligner _aligner;

public:
    /**
     * @brief Constructs a Parking manager from a configuration file.
     *
     * The constructor:
     *  - loads parking place geometries from a JSON file,
     *  - initializes all ParkingPlace instances,
     *  - stores the reference frame used for alignment.
     *
     * @param jsonPath Path to the JSON file describing parking places.
     * @param reference Reference image used for frame alignment.
     */
    Parking(const std::string& jsonPath, cv::Mat reference);

    /**
     * @brief Returns the total number of parking places.
     *
     * @return Total number of managed parking places.
     */
    size_t getNumPlace() const;

    /**
     * @brief Returns the number of currently occupied parking places.
     *
     * This value reflects the result of the last evolve() call.
     *
     * @return Number of occupied parking places.
     */
    int getNumOccupied() const;

    /**
     * @brief Returns last FPS.
     *
     * This value reflects the result of the last evolve() call.
     *
     * @return Current FPS.
     */
    double getFps() const;

    /**
     * @brief Returns last latency measured in ms.
     *
     * This value reflects the result of the last evolve() call.
     *
     * @return Last latency of evolve function.
     */
    double getLastLatencyMs() const;

    /**
     * @brief Return system metrics converted to json 
     *
     * This value reflects the result of the last evolve() call.
     *
     * @return Last evaluation of evolve function into a json file.
     */
    std::map<std::string, double> getStats() const;

    /**
     * @brief Updates the state of the parking using a new video frame.
     *
     * The method:
     *  - aligns the current frame to the reference image,
     *  - updates each ParkingPlace state,
     *  - recomputes global occupancy statistics.
     *
     * @param frame Current video frame.
     */
    void evolve(const Frame& frame);

    /**
     * @brief Returns rendering data for all parking places.
     *
     * This method exposes lightweight, rendering-oriented data structures
     * that can be consumed by visualization layers (OpenCV overlays, UI,
     * bindings, etc.) without exposing internal logic.
     *
     * @return Vector of RenderPlace objects.
     */
    std::vector<RenderPlace> getRenderData() const;

    bool hasAffine() const;
    std::array<double, 6> getAffine() const;
    RenderSnapshot getRenderSnapshot() const;
};
