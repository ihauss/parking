#include "Parking.h"

Parking::Parking(const std::string& jsonPath, cv::Mat reference)
    : _numOccupied(0)
{
    // Open JSON configuration file
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open JSON file: " << jsonPath << std::endl;
        return;
    }

    // Parse JSON content
    nlohmann::json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
        return;
    }

    // Validate JSON structure
    if (!j.contains("parking_places") || !j["parking_places"].is_array()) {
        std::cerr << "JSON format error: missing 'parking_places' array" << std::endl;
        return;
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
        cv::Point coords_arr[4];
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
        if (place.getState() == OCCUPIED)
            ++_numOccupied;
    }

    // Store reference image and initialize tracking
    _reference = reference;
    initReference();
}

void Parking::initReference()
{
    cv::Mat gray;
    cv::cvtColor(_reference, gray, cv::COLOR_BGR2GRAY);

    // Detect strong corners to track
    cv::goodFeaturesToTrack(
        gray,
        _refPts,
        500,   // maximum number of corners
        0.01,  // quality level
        10     // minimum distance between points
    );

    _prevGray = gray.clone();
    _prevPts  = _refPts;
    _flowInitialized = true;
}

size_t Parking::getNumPlace() const {
    return _places.size();
}

int Parking::getNumOccupied() const {
    return _numOccupied;
}

void Parking::evolve(const cv::Mat& frame) {
    int newCount = 0;

    for (auto& place : _places) {
        place.changeState(frame);

        if (place.getState() == OCCUPIED ||
            place.getState() == TRANSITION_OUT)
        {
            newCount++;
        }
    }

    _numOccupied = newCount;
}

void Parking::drawParking(cv::Mat& frame){
    cv::Mat overlay = frame.clone();

    for (auto& place : _places) {
        place.drawPlace(frame, overlay);
    }

    // Blend overlay with original frame
    cv::addWeighted(frame, 0.6, overlay, 0.4, 0.0, frame);
}

bool Parking::alignToReference(const cv::Mat& frame, cv::Mat& warped){
    if (!_flowInitialized)
        return false;

    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    std::vector<cv::Point2f> currPts;
    std::vector<uchar> status;
    std::vector<float> err;

    // Track reference points using optical flow
    cv::calcOpticalFlowPyrLK(
        _prevGray,
        gray,
        _prevPts,
        currPts,
        status,
        err
    );

    // Keep only valid tracked points
    std::vector<cv::Point2f> src, dst;
    for (size_t i = 0; i < status.size(); ++i) {
        if (status[i]) {
            src.push_back(currPts[i]);  // current frame points
            dst.push_back(_refPts[i]);  // reference points
        }
    }

    if (src.size() < 10)
        return false;

    // Estimate affine transformation
    std::vector<uchar> inliers;
    cv::Mat A = cv::estimateAffinePartial2D(
        src,
        dst,
        inliers,
        cv::RANSAC
    );

    if (A.empty())
        return false;

    // Warp frame to reference coordinates
    cv::warpAffine(frame, warped, A, _reference.size());

    // Update tracking state
    _prevGray = gray.clone();
    _prevPts  = currPts;

    return true;
}

void Parking::addBanner(const cv::Mat& frame, cv::Mat& output, const double& fps){
    int W = frame.cols;
    int H = frame.rows;

    // Banner height (12% of frame height)
    int bannerH = static_cast<int>(0.12 * H);

    cv::Mat banner(
        bannerH,
        W,
        frame.type(),
        cv::Scalar(30, 30, 30)
    );

    int free = getNumPlace() - getNumOccupied();

    std::string fpsText   = "FPS: " + std::to_string(static_cast<int>(fps));
    std::string ratioText = "Free : " +
                            std::to_string(free) + "/" +
                            std::to_string(getNumPlace());

    int font = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = bannerH / 60.0;
    int thickness = 2;
    cv::Scalar color(255, 255, 255);

    int baseline = 0;
    cv::Size textSize = cv::getTextSize(
        ratioText,
        font,
        fontScale,
        thickness,
        &baseline
    );

    int x = W - textSize.width - 20;
    int y = (bannerH - (textSize.height + baseline)) / 2 + textSize.height;

    cv::putText(banner, fpsText, cv::Point(20, y),
                font, fontScale, color, thickness, cv::LINE_AA);

    cv::putText(banner, ratioText, cv::Point(x, y),
                font, fontScale, color, thickness, cv::LINE_AA);

    // Concatenate banner and frame
    cv::vconcat(banner, frame, output);

    // Resize back to original frame size
    cv::resize(output, output, frame.size(), 0, 0, cv::INTER_LINEAR);
}
