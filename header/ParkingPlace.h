#ifndef PARKING_PLACE_H
#define PARKING_PLACE_H

#include <opencv2/opencv.hpp>
#include <future>

enum placeState {
    FREE,
    TRANSITION_IN,
    OCCUPIED,
    TRANSITION_OUT
};

/**
 * @class ParkingPlace
 * @brief Represents a single parking slot and manages its occupancy state.
 *
 * The ParkingPlace class models one parking spot defined by four corner points.
 * It provides methods to:
 *  - evaluate occupancy using appearance-based analysis,
 *  - detect motion using background subtraction,
 *  - handle transient states during vehicle entry and exit,
 *  - draw visual annotations on video frames.
 *
 * State transitions are handled asynchronously to avoid blocking the main
 * processing loop.
 */
class ParkingPlace {
private:
    int _id;
    cv::Point _coords[4];   // Les 4 points de la place
    placeState _state;
    std::future<placeState> _futureState;  
    cv::Mat _mask;
    cv::Ptr<cv::BackgroundSubtractorKNN> _knn;
    bool _coordAdjust = false;
    unsigned int _warmUp = 50;

    /**
     * @brief Clamps parking coordinates to remain within frame boundaries.
     *
     * If coordinates exceed the frame size, they are adjusted and the
     * internal mask is recomputed.
     *
     * @param frameSize Size of the current video frame.
     * @return True if any coordinate was modified.
     */
    bool adjustCoords(const cv::Size& frameSize);

    /**
     * @brief Detects motion inside the parking place region.
     *
     * Motion detection is performed using a KNN background subtractor
     * restricted to the parking mask.
     *
     * @param frame Current video frame.
     * @param thresh Motion ratio threshold.
     * @return True if motion exceeds the given threshold.
     */
    bool hasMovement(const cv::Mat& frame, double thresh);

    /**
     * @brief Initializes the binary mask corresponding to the parking polygon.
     *
     * The mask is used to restrict motion detection and appearance analysis
     * strictly to the parking area.
     */
    void initMask();

    /**
     * @brief Evaluates the occupancy state of the parking place.
     *
     * The method applies a perspective warp to normalize the parking region,
     * converts it to YCrCb color space, and estimates luminance variance
     * to infer whether the place is occupied.
     *
     * @param frame Current video frame.
     * @return Estimated occupancy state.
     */
    placeState evalState(const cv::Mat& frame);

public:
    /**
     * @brief Constructs a ParkingPlace from four corner coordinates.
     *
     * Initializes internal geometry, occupancy mask, and background
     * subtraction model.
     *
     * @param coords Array of four points defining the parking slot polygon.
     * @param id Unique identifier of the parking place.
     */
    ParkingPlace(const cv::Point coords[4], int id = 0);

    /**
     * @brief Returns the current state of the parking place.
     */
    placeState getState() const;

    /**
     * @brief Returns the polygon coordinates defining the parking place.
     *
     * @return Pointer to an array of four cv::Point.
     */
    const cv::Point* getCoords() const;

    /**
     * @brief Updates the parking place state based on motion and appearance.
     *
     * This method:
     *  - launches asynchronous state evaluation when needed,
     *  - manages transient states (TRANSITION_IN / TRANSITION_OUT),
     *  - finalizes occupancy state once motion stabilizes.
     *
     * @param frame Current video frame.
     */
    void changeState(const cv::Mat& frame);

    /**
     * @brief Draws the parking place contour and fill on the output frame.
     *
     * Color coding reflects the current occupancy state.
     *
     * @param frame Frame where contours are drawn.
     * @param overlay Semi-transparent overlay for filled polygons.
     */
    void drawPlace(cv::Mat& frame, cv::Mat& overlay);
};

#endif
