#include <gtest/gtest.h>

#include "smart_parking/Parking.h"
#include <opencv2/opencv.hpp>

// initialization file not exist
TEST(ParkingConstructor, ThrowsIfFileDoesNotExist) {
    cv::Mat ref = cv::Mat::zeros(100, 100, CV_8UC3);

    EXPECT_THROW(
        Parking("non_existing_file.json", ref),
        std::runtime_error
    );
}

// initialization file invalid
TEST(ParkingConstructor, ThrowsOnInvalidJson) {
    cv::Mat ref = cv::Mat::zeros(100, 100, CV_8UC3);

    EXPECT_THROW(
        Parking("tests/data/invalid_json.json", ref),
        std::runtime_error
    );
}

// initialization file with no parking places
TEST(ParkingConstructor, ThrowsIfParkingPlacesMissing) {
    cv::Mat ref = cv::Mat::zeros(100, 100, CV_8UC3);

    EXPECT_THROW(
        Parking("tests/data/missing_parking_places.json", ref),
        std::runtime_error
    );
}

// initialization file is correct
TEST(ParkingConstructor, DoesNotThrowOnValidConfig) {
    cv::Mat ref = cv::Mat::zeros(100, 100, CV_8UC3);

    EXPECT_NO_THROW(
        Parking("tests/data/valid_parking.json", ref)
    );
}

// test number of place
TEST(getNumPlace, LoadsCorrectNumberOfPlaces) {
    cv::Mat ref = cv::Mat::zeros(100, 100, CV_8UC3);

    Parking p("tests/data/valid_parking.json", ref);

    EXPECT_EQ(p.getNumPlace(), 3);
}
