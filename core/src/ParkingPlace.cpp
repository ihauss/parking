#include "smart_parking/ParkingPlace.h"

using namespace std::chrono_literals;

// Constructor: initialize parking place with coordinates and unique ID
ParkingPlace::ParkingPlace(const std::array<cv::Point, 4> coords, int id)
    : _id(id), _currentState(PlaceState::INIT_STATE)
{
    // Copy quadrilateral coordinates
    for (int i = 0; i < 4; i++) {
        _coords[i] = coords[i];
    }
}

// Returns current state of the parking place
PlaceState ParkingPlace::getState() const{
    return _currentState;
}

// Checks if the last heavy estimation is still within cooldown window
bool ParkingPlace::isRecent(std::chrono::steady_clock::time_point timestamp, std::chrono::milliseconds coolDownTime){
    bool recent = timestamp - _lastEstimationTime < coolDownTime;
    return recent;
}

// FSM update logic
// Returns true if a heavy estimation is required
bool ParkingPlace::update(LightVisionData& data, std::optional<bool> info){

    // If heavy estimator returned a result, update state immediately
    if(info.has_value()){
        if(*info)_currentState = PlaceState::OCCUPIED;
        else _currentState = PlaceState::FREE;
        _lastEstimationTime = data.timestamp;
    }

    // Prevent too frequent heavy estimations
    if(isRecent(data.timestamp, 500ms))return false;

    switch (_currentState)
    {
    case PlaceState::INIT_STATE:
        // First estimation required at startup
        _lastEstimationTime = data.timestamp;
        return true;

    case PlaceState::FREE:
        // Motion detected on free spot → possible car entering
        if(data.hasMovement)_currentState = PlaceState::TRANSITION_IN;
        break;

    case PlaceState::OCCUPIED:
        // Motion detected on occupied spot → possible car leaving
        if(data.hasMovement)_currentState = PlaceState::TRANSITION_OUT;
        break;

    default:
        // If in transition and movement stops → request heavy confirmation
        if(!data.hasMovement)return true;
        break;
    }
    return false;
}

// Returns quadrilateral coordinates
const std::array<cv::Point, 4> ParkingPlace::getCoords() const {
    return _coords;
}

// Ensures coordinates stay inside frame boundaries
bool ParkingPlace::adjustCoords(cv::Size& frameSize)
{
    bool modified = false;

    for (int i = 0; i < 4; ++i) {
        int x = _coords[i].x;
        int y = _coords[i].y;

        int clampedX = std::clamp(x, 0, frameSize.width  - 1);
        int clampedY = std::clamp(y, 0, frameSize.height - 1);

        if (x != clampedX || y != clampedY) {
            _coords[i].x = clampedX;
            _coords[i].y = clampedY;
            modified = true;
        }
    }
    _coordAdjust = true;

    return modified;
}

// Main state evolution per frame
void ParkingPlace::changeState(cv::Mat& frame) {

    // If asynchronous heavy estimation finished → retrieve result
    if (_occupancyResultAsc.valid() && _occupancyResultAsc.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            _occupancyResult = _occupancyResultAsc.get();
    }

    // Ensure coordinates are valid
    cv::Size frameSize = frame.size();
    if(!_coordAdjust)adjustCoords(frameSize);

    // Run lightweight motion detector
    struct LightVisionData data = _lightVision(frame, _coords, 0.25);

    // Update FSM and determine if heavy estimation is required
    bool needHeavyEstimation = update(data, _occupancyResult);

    // Launch heavy estimator asynchronously if needed
    if(needHeavyEstimation){
        _occupancyResultAsc = std::async(std::launch::async, 
            [this, frame]() mutable 
            {return _estimator(frame, _coords);}
        );
    }

    // Reset last heavy result to avoid reusing stale data
    _occupancyResult.reset();
}