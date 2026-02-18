#include "smart_parking/Parking.h"

// Constructor loading parking configuration and initializing components
Parking::Parking(const std::string& jsonPath, cv::Mat reference)
    : _numOccupied(0), _aligner(reference)
{
    // Open JSON configuration file
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open JSON file: " + jsonPath);
    }

    // Parse JSON content
    nlohmann::json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        throw std::runtime_error(
            std::string("Failed to parse JSON: ") + e.what()
        );
    }

    // Validate JSON structure
    if (!j.contains("parking_places") || !j["parking_places"].is_array()) {
        throw std::runtime_error("Invalid JSON format: missing parking_places");
    }

    int id_counter = 0;

    // Load each parking place definition
    for (const auto& item : j["parking_places"]) {
        id_counter++;

        // Check that coordinates exist and form a quadrilateral
        if (!item.contains("coords") || !item["coords"].is_array() || item["coords"].size() != 4) {
            std::cerr << "Skipping place " << id_counter << " : 'coords' missing or not length 4\n";
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
            std::cerr << "Skipping place " << id_counter << " : malformed point\n";
            continue;
        }

        // Create a parking place and add it to the parking
        ParkingPlace place(coords_arr, id_counter);
        _places.push_back(std::move(place));
    }

    // Count initially occupied places
    _numOccupied = 0;
    for (auto& place : _places) {
        if (place.getState() == PlaceState::OCCUPIED)
            ++_numOccupied;
    }
}

// Returns total number of parking places
size_t Parking::getNumPlace() const {
    return _places.size();
}

// Returns number of currently occupied places
int Parking::getNumOccupied() const {
    return _numOccupied;
}

// Updates parking states for a new video frame
void Parking::evolve(cv::Mat& frame) {

    // Align current frame with the reference image
    cv::Mat output;
    if (!_aligner(frame, output)) return;

    int newCount = 0;

    // Update state of each parking place
    for (auto& place : _places) {
        place.changeState(output);

        // Count occupied and exiting vehicles
        if (place.getState() == PlaceState::OCCUPIED ||
            place.getState() == PlaceState::TRANSITION_OUT)
        {
            newCount++;
        }
    }

    // Update global occupancy count
    _numOccupied = newCount;

    // Render visualization and overlay statistics
    _renderer(output, getRenderData(), frame, _numOccupied, getNumPlace());
}

// Collect rendering data for all parking places
std::vector<RenderPlace> Parking::getRenderData() const {
    std::vector<RenderPlace> out;
    out.reserve(_places.size());

    // Convert internal state to renderable structures
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
