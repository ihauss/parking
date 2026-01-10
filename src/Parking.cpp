#include "Parking.h"

Parking::Parking(const std::string& jsonPath, cv::Mat reference)
    : _numOccupied(0)
{
    // ouvrir le fichier
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open JSON file: " << jsonPath << std::endl;
        return;
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
        return;
    }

    int id_counter = 0;
    if (!j.contains("parking_places") || !j["parking_places"].is_array()) {
        std::cerr << "JSON format error: missing 'parking_places' array" << std::endl;
        return;
    }

    for (const auto& item : j["parking_places"]) {
        id_counter++;
        // sécurité : vérifier que coords existe et est un tableau de 4 éléments
        if (!item.contains("coords") || !item["coords"].is_array() || item["coords"].size() != 4) {
            std::cerr << "Skipping place " << id_counter << " : 'coords' missing or not length 4\n";
            continue;
        }

        // remplir le tableau de cv::Point
        cv::Point coords_arr[4];
        bool coords_ok = true;
        for (int i = 0; i < 4; ++i) {
            const auto& p = item["coords"][i];
            if (!p.contains("x") || !p.contains("y")) { coords_ok = false; break; }
            coords_arr[i].x = p["x"].get<int>();
            coords_arr[i].y = p["y"].get<int>();
        }
        if (!coords_ok) {
            std::cerr << "Skipping place " << id_counter << " : malformed point\n";
            continue;
        }

        // créer la place (appel au constructeur qui doit copier le tableau)
        ParkingPlace place(coords_arr, id_counter);
        _places.push_back(std::move(place));
    }

    // Initialiser le nombre de places occupées
    _numOccupied = 0;
    for (auto& place : _places) {
        if (place.getState() == OCCUPIED)
            ++_numOccupied;
    }
    _reference = reference;
    initReference();
}

void Parking::initReference()
{
    cv::Mat gray;
    cv::cvtColor(_reference, gray, cv::COLOR_BGR2GRAY);

    // Détecter de bons points à suivre
    cv::goodFeaturesToTrack(
        gray,
        _refPts,
        500,        // max corners
        0.01,       // quality
        10          // min distance
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
        placeState prev = place.getState();
        place.changeState(frame);

        if (place.getState() == OCCUPIED || place.getState() == TRANSITION_OUT)
            newCount++;
    }

    _numOccupied = newCount;
}

void Parking::drawParking(cv::Mat& frame){
    cv::Mat overlay = frame.clone();

    for (auto& place : _places) {
        place.drawPlace(frame, overlay);
    }

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
    // Optical Flow (KLT)
    cv::calcOpticalFlowPyrLK(
        _prevGray,
        gray,
        _prevPts,
        currPts,
        status,
        err
    );

    // Filtrer les points valides
    std::vector<cv::Point2f> src, dst;
    for (size_t i = 0; i < status.size(); ++i) {
        if (status[i]) {
            src.push_back(currPts[i]);   // frame courante
            dst.push_back(_refPts[i]);   // référence
        }
    }

    if (src.size() < 10)
        return false;

    std::vector<uchar> inliers;
    cv::Mat A = cv::estimateAffinePartial2D(
        src,
        dst,
        inliers,
        cv::RANSAC
    );

    if (A.empty())
        return false;

    cv::warpAffine(frame, warped, A, _reference.size());
    // Mise à jour pour la frame suivante
    _prevGray = gray.clone();
    _prevPts  = currPts;

    return true;
}

void Parking::addBanner(const cv::Mat& frame, cv::Mat& output, const double& fps){
    int W = frame.cols;
    int H = frame.rows;

    int bannerH = static_cast<int>(0.12 * H); // 12% de la hauteur

    cv::Mat banner(
        bannerH,
        W,
        frame.type(),
        cv::Scalar(30, 30, 30) // gris foncé
    );

    int free = getNumPlace()-getNumOccupied();

    std::string fpsText = "FPS: " + std::to_string(static_cast<int>(fps));
    std::string ratioText = "Free : " +
                            std::to_string(free) + "/" +
                            std::to_string(getNumPlace());

    int font = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = bannerH / 60.0;
    int thickness = 2;
    cv::Scalar color(255, 255, 255); // blanc

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

    cv::putText(
        banner,
        fpsText,
        cv::Point(20, y),
        font,
        fontScale,
        color,
        thickness,
        cv::LINE_AA
    );

    cv::putText(
        banner,
        ratioText,
        cv::Point(x, y),
        font,
        fontScale,
        color,
        thickness,
        cv::LINE_AA
    );

    cv::vconcat(banner, frame, output);
    cv::resize(output, output, cv::Size(frame.cols ,frame.rows), 0, 0, cv::INTER_LINEAR);

}