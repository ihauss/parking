#include <gtest/gtest.h>

#include "smart_parking/ParkingPlace.h"
#include <opencv2/opencv.hpp>

// Test if the initial state is correct
TEST(getState, CorrectInitialState) {
    cv::Point coords[4] = {
        cv::Point(0, 0),
        cv::Point(0, 0),
        cv::Point(0, 0),
        cv::Point(0, 0)
    };
    
    ParkingPlace pp(coords, 0);
    EXPECT_EQ(pp.getState(), FREE);
}

// Test if the initial coords are corrects
TEST(getCoords, CorrectInitialCoords) {
    cv::Point coords[4] = {
        cv::Point(0, 0),
        cv::Point(0, 0),
        cv::Point(0, 0),
        cv::Point(0, 0)
    };

    ParkingPlace pp(coords, 0);
    for(int i = 0; i < 4; i++){
        EXPECT_EQ(pp.getCoords()[i].x, coords[i].x);
        EXPECT_EQ(pp.getCoords()[i].y, coords[i].y);
    }
}

// Test the case where the coordinates are all in the image
TEST(adjustCoords, NoClamp) {
    cv::Point coords[4] = {
        cv::Point(0, 0),
        cv::Point(0, 0),
        cv::Point(0, 0),
        cv::Point(0, 0)
    };

    cv::Size s(50, 50);
    ParkingPlace pp(coords, 0);
    pp.adjustCoords(s);
    for(int i = 0; i < 4; i++){
        EXPECT_EQ(pp.getCoords()[i].x, coords[i].x);
        EXPECT_EQ(pp.getCoords()[i].y, coords[i].y);
    }
}

// Test the case where the top coordinates are off grid
TEST(adjustCoords, TopClamp) {
    cv::Point coords[4] = {
        cv::Point(-5, 0),
        cv::Point(-5, 0),
        cv::Point(0, 0),
        cv::Point(0, 0)
    };

    cv::Size s(50, 50);
    ParkingPlace pp(coords, 0);
    pp.adjustCoords(s);
    EXPECT_EQ(pp.getCoords()[0].x, 0);
    EXPECT_EQ(pp.getCoords()[1].x, 0);
}

// Test the case where the bottom coordinates are off grid
TEST(adjustCoords, BottomClamp) {
    cv::Point coords[4] = {
        cv::Point(0, 0),
        cv::Point(0, 0),
        cv::Point(55, 0),
        cv::Point(55, 0)
    };

    cv::Size s(50, 50);
    ParkingPlace pp(coords, 0);
    pp.adjustCoords(s);
    EXPECT_EQ(pp.getCoords()[2].x, 49);
    EXPECT_EQ(pp.getCoords()[3].x, 49);
}

// Test the case where the left coordinates are off grid
TEST(adjustCoords, LeftClamp) {
    cv::Point coords[4] = {
        cv::Point(0, -5),
        cv::Point(0, 0),
        cv::Point(0, 0),
        cv::Point(0, -5)
    };

    cv::Size s(50, 50);
    ParkingPlace pp(coords, 0);
    pp.adjustCoords(s);
    EXPECT_EQ(pp.getCoords()[0].y, 0);
    EXPECT_EQ(pp.getCoords()[3].y, 0);
}

// Test the case where the right coordinates are off grid
TEST(adjustCoords, RightClamp) {
    cv::Point coords[4] = {
        cv::Point(0, 0),
        cv::Point(0, 55),
        cv::Point(0, 55),
        cv::Point(0, 0)
    };

    cv::Size s(50, 50);
    ParkingPlace pp(coords, 0);
    pp.adjustCoords(s);
    EXPECT_EQ(pp.getCoords()[1].y, 49);
    EXPECT_EQ(pp.getCoords()[2].y, 49);
}