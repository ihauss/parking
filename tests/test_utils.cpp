#include <gtest/gtest.h>

#include "utils.h"
#include <opencv2/opencv.hpp>

// Test no args
TEST(parseArgs, Default) {
    int argc = 1;
    const char* argv[] ={"smart_parking"};

    AppConfig config = parseArgs(argc, const_cast<char**>(argv));
    EXPECT_FALSE(config.headless);
    EXPECT_FALSE(config.record);
    EXPECT_EQ(config.outputPath, "output.mp4");
}

// Test headless arg
TEST(parseArgs, Headless) {
    int argc = 2;
    const char* argv[] = {"smart_parking", "--headless"};

    AppConfig config = parseArgs(argc, const_cast<char**>(argv));
    EXPECT_TRUE(config.headless);
    EXPECT_FALSE(config.record);
    EXPECT_EQ(config.outputPath, "output.mp4");
}

// Test rec arg
TEST(parseArgs, Rec) {
    int argc = 2;
    const char* argv[] = {"smart_parking", "--rec"};

    AppConfig config = parseArgs(argc, const_cast<char**>(argv));
    EXPECT_FALSE(config.headless);
    EXPECT_TRUE(config.record);
    EXPECT_EQ(config.outputPath, "output.mp4");
}

// Test output arg
TEST(parseArgs, Output) {
    int argc = 4;
    const char* argv[] = {"smart_parking", "--rec", "--output", "custom_output.mp4"};

    AppConfig config = parseArgs(argc, const_cast<char**>(argv));
    EXPECT_FALSE(config.headless);
    EXPECT_TRUE(config.record);
    EXPECT_EQ(config.outputPath, "custom_output.mp4");

}

// Test incorrect arg
TEST(parseArgs, Incorrect) {
    int argc = 2;
    const char* argv[] ={"smart_parking", "--record"};

    AppConfig config = parseArgs(argc, const_cast<char**>(argv));
    EXPECT_FALSE(config.headless);
    EXPECT_FALSE(config.record);
    EXPECT_EQ(config.outputPath, "output.mp4");
}