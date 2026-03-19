#include "smart_parking/Parking.h"

namespace smart_parking {

// Constructor loading parking configuration and initializing components
Parking::Parking(const std::string& jsonPath, const cv::Mat& reference)
    : _numOccupied(0), _aligner(reference)
{
    // Open JSON configuration file
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        Logger::log().error("Failed to open JSON file: " + jsonPath);
        throw std::runtime_error("Failed to open JSON file: " + jsonPath);
    }

    // Parse JSON content
    nlohmann::json j;
    try {
        file >> j;
    }
    catch (const std::exception& e) {
        Logger::log().error(std::string("Failed to parse JSON: ") + e.what());
        throw std::runtime_error(
            std::string("Failed to parse JSON: ") + e.what()
        );
    }

    // Validate JSON structure
    if (!j.contains("parking_places") || !j["parking_places"].is_array()) {
        Logger::log().error("Invalid JSON format: missing parking_places");
        throw std::runtime_error("Invalid JSON format: missing parking_places");
    }

    int id_counter = 0;

    // Load each parking place definition
    for (const auto& item : j["parking_places"]) {
        id_counter++;

        // Check that coordinates exist and form a quadrilateral
        if (!item.contains("coords") || !item["coords"].is_array() || item["coords"].size() != 4) {
            Logger::log().warn(
                "Skipping place " + std::to_string(id_counter) +
                ": 'coords' missing or not length 4"
            );
            continue;
        }

        // Convert JSON coordinates to OpenCV points
        std::array<cv::Point, 4> coords_arr;
        bool coords_ok = true;

        for (int i = 0; i < 4; ++i) {
            const auto& p = item["coords"][i];

            if (!p.contains("x") || !p.contains("y")) {
                coords_ok = false;
                break;
            }

            coords_arr[i].x = p["x"].get<int>();
            coords_arr[i].y = p["y"].get<int>();
        }

        // Skip malformed parking place definitions
        if (!coords_ok) {
            Logger::log().warn(
                "Skipping place " + std::to_string(id_counter) +
                ": malformed coordinates"
            );
            continue;
        }

        // Create a parking place and add it to the parking manager
        ParkingPlace place(coords_arr, id_counter);
        _places.push_back(std::move(place));
    }

    // Initial occupancy count (should normally be zero at startup)
    _numOccupied = 0;
    for (auto& place : _places) {
        if (place.getState() == PlaceState::OCCUPIED)
            ++_numOccupied;
    }

    Logger::log().info(
        "Parking initialized with " +
        std::to_string(_places.size()) + " places"
    );
}

// Returns total number of parking places
size_t Parking::getNumPlace() const {
    return _places.size();
}

// Returns number of currently occupied places
int Parking::getNumOccupied() const {
    return _numOccupied;
}

// Returns estimated FPS based on last measured latency
double Parking::getFps() const {
    if (_lastLatencyMs <= 0.0)
        return 0.0;

    return 1000.0 / _lastLatencyMs;
}

// Returns last measured latency (ms)
double Parking::getLastLatencyMs() const {
    return _lastLatencyMs;
}

// Returns aggregated system metrics as a key-value map
std::map<std::string, double> Parking::getStats() const {
    return {
        {"fps", getFps()},
        {"latency_ms", getLastLatencyMs()},
        {"occupied", static_cast<double>(getNumOccupied())},
        {"free", static_cast<double>(getNumPlace() - getNumOccupied())},
        {"total", static_cast<double>(getNumPlace())}
    };
}

// Updates parking states for a new video frame
void Parking::evolve(const Frame& frame){
    // Start latency measurement
    auto t0 = std::chrono::steady_clock::now();

    // Validate input frame
    if (!frame.isValid()) {
        _state = CameraState::ERROR;
        Logger::log().error("Invalid frame received");
        throw std::runtime_error("Invalid frame format");
    }

    _state = CameraState::RUNNING;

    const cv::Mat& input = frame.data;

    // Align frame to reference coordinate system
    cv::Mat aligned;
    if (!_aligner(input, aligned)) {
        Logger::log().warn("Frame alignment failed, skipping frame");
        return;
    }

    int newCount = 0;

    // Update each parking place state
    for (auto& place : _places) {
        place.changeState(aligned);

        // Count occupied or transitioning-out places as occupied
        if (place.getState() == PlaceState::OCCUPIED ||
            place.getState() == PlaceState::TRANSITION_OUT)
        {
            newCount++;
        }
    }

    // Update global occupancy count
    _numOccupied = newCount;

    // End latency measurement
    auto t1 = std::chrono::steady_clock::now();
    _lastLatencyMs =
        std::chrono::duration<double, std::milli>(t1 - t0).count();
}

// Collect rendering data for all parking places
std::vector<RenderPlace> Parking::getRenderData() const {
    std::vector<RenderPlace> out;
    out.reserve(_places.size());

    // Convert internal state to rendering-friendly format
    for (const auto& place : _places) {
        out.push_back({
            { place.getCoords()[0],
              place.getCoords()[1],
              place.getCoords()[2],
              place.getCoords()[3] },
            place.getState()
        });
    }

    return out;
}

// Indicates whether a valid affine transform is available
bool Parking::hasAffine() const {
    return _aligner.hasAffine();
}

// Returns last affine transform (flattened 2x3 matrix)
std::array<double, 6> Parking::getAffine() const {
    const cv::Mat& A = _aligner.getAffine();

    return {
        A.at<double>(0,0), A.at<double>(0,1), A.at<double>(0,2),
        A.at<double>(1,0), A.at<double>(1,1), A.at<double>(1,2)
    };
}

// Returns full rendering snapshot (UI / API ready)
smart_parking::RenderSnapshot Parking::getRenderSnapshot() const{
    smart_parking::RenderSnapshot snapshot;

    // Copy renderable places
    snapshot.places = getRenderData();

    // Affine transform availability
    snapshot.hasAffine = hasAffine();
    if (snapshot.hasAffine) {
        snapshot.affine = getAffine();
    }

    // Global stats
    snapshot.numOccupied = _numOccupied;
    snapshot.numPlaces = getNumPlace();

    return snapshot;
}

// Returns current camera/system state
CameraState Parking::getState() const {
    return _state;
}

} // namespace smart_parking