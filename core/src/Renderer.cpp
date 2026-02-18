#include "smart_parking/Renderer.h"

/**
 * @brief Color palette used to render parking places.
 *
 * The colors are indexed according to PlaceState order
 * (excluding INIT_STATE, which is never rendered).
 *
 * Mapping:
 *  - FREE            : Green
 *  - TRANSITION_IN   : Orange
 *  - OCCUPIED        : Red
 *  - TRANSITION_OUT  : Yellow
 */
const std::array<cv::Scalar, 4> COLORS = {
    cv::Scalar(0, 255, 0),    // FREE
    cv::Scalar(0, 127, 255),  // TRANSITION_IN
    cv::Scalar(0, 0, 255),    // OCCUPIED
    cv::Scalar(0, 255, 255)   // TRANSITION_OUT
};

//Update the internal FPS estimation using an exponential moving average.
void Renderer::updateFPS(double alpha){
    // Measure elapsed time since last frame
    auto tNow = Clock::now();
    double dt = std::chrono::duration<double>(tNow - _tPrev).count();
    _tPrev = tNow;

    // Instantaneous FPS
    double currentFps = 1.0 / dt;

    // Exponential moving average for smoother FPS display
    _fps = (_fps == 0.0)
        ? currentFps
        : alpha * currentFps + (1.0 - alpha) * _fps;
}

//Draw parking places on the frame.
void Renderer::draw(cv::Mat& frame, const std::vector<RenderPlace>& places){
    // Overlay used for semi-transparent filling
    cv::Mat overlay = frame.clone();

    for (const auto& p : places) {
        std::vector<std::vector<cv::Point>> contour;
        contour.emplace_back(p.coords.begin(), p.coords.end());

        // Select color according to parking state
        cv::Scalar color = COLORS[static_cast<int>(p.state) - 1];

        // Draw outline
        cv::polylines(frame, contour, true, color, 3, cv::LINE_AA);

        // Fill overlay
        cv::fillPoly(overlay, contour, color, cv::LINE_AA);
    }

    // Blend overlay with original frame
    cv::addWeighted(frame, 0.6, overlay, 0.4, 0.0, frame);
}

//Add an informational banner on top of the frame.
void Renderer::addBanner(
    const cv::Mat& frame, 
    cv::Mat& output,
    int numOccupied,
    int numPlace
){
    int W = frame.cols;
    int H = frame.rows;

    // Banner height set to 12% of frame height
    int bannerH = static_cast<int>(0.12 * H);

    // Create banner background
    cv::Mat banner(
        bannerH,
        W,
        frame.type(),
        cv::Scalar(30, 30, 30)
    );

    int free = numPlace - numOccupied;

    // Text content
    std::string fpsText   = "FPS: " + std::to_string(static_cast<int>(_fps));
    std::string ratioText = "Free : " +
                            std::to_string(free) + "/" +
                            std::to_string(numPlace);

    // Font configuration
    int font = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = bannerH / 60.0;
    int thickness = 2;
    cv::Scalar color(255, 255, 255);

    // Compute text alignment
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

    // Draw FPS (left)
    cv::putText(banner, fpsText, cv::Point(20, y),
                font, fontScale, color, thickness, cv::LINE_AA);

    // Draw free/total ratio (right)
    cv::putText(banner, ratioText, cv::Point(x, y),
                font, fontScale, color, thickness, cv::LINE_AA);

    // Stack banner and frame vertically
    cv::vconcat(banner, frame, output);

    // Resize back to original frame size
    cv::resize(output, output, frame.size(), 0, 0, cv::INTER_LINEAR);
}

//Main rendering entry point.
void Renderer::operator()(
    cv::Mat& frame, 
    const std::vector<RenderPlace>& places, 
    cv::Mat& output,
    int numOccupied,
    int numPlace
){
    // Draw parking slots and occupancy status
    draw(frame, places);

    // Update FPS estimation
    updateFPS(0.1);

    // Add informational overlay (FPS, statistics, etc.)
    addBanner(frame, output, numOccupied, numPlace);

    // Replace input frame with rendered output
    frame = output;
}
