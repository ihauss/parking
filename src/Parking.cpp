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
        _places.push_back(place);
    }

    // Initialiser le nombre de places occupées
    _numOccupied = 0;
    for (auto& place : _places) {
        if (place.getState() == OCCUPIED)
            ++_numOccupied;
    }
    _reference = reference;
    _orb = cv::ORB::create(1000);
    _orb->detectAndCompute(_reference, cv::noArray(), _kr, _dr);
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
    std::vector<cv::KeyPoint> kf;
    cv::Mat df;

    _orb->detectAndCompute(frame, cv::noArray(), kf, df);

    if (df.empty() || _dr.empty()) return false;
    cv::BFMatcher matcher(cv::NORM_HAMMING);
    std::vector<std::vector<cv::DMatch>> knn;
    matcher.knnMatch(df, _dr, knn, 2);

    std::vector<cv::DMatch> good;
    const float ratio_thresh = 0.75f;
    for (auto &m : knn) {
        if (m.size() >= 2 && m[0].distance < ratio_thresh * m[1].distance)
            good.push_back(m[0]);
    }
    if (good.size() < 10) return false;

    std::vector<cv::Point2f> pts_src, pts_dst;
    for (const auto &m : good) {
        pts_src.push_back(kf[m.queryIdx].pt); // frame
        pts_dst.push_back(_kr[m.trainIdx].pt); // reference
    }

    std::vector<unsigned char> inliersMask;
    cv::Mat A = cv::estimateAffinePartial2D(pts_src, pts_dst, inliersMask);

    if (A.empty()) return false;

    int inliers = std::count(inliersMask.begin(), inliersMask.end(), 1);
    if (inliers < 8) return false;

    cv::warpAffine(frame, warped, A, _reference.size());

    return true;
}
