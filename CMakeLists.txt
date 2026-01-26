cmake_minimum_required(VERSION 3.15)
project(smart_parking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Warnings = -Wall -Wextra -Werror
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

find_package(OpenCV REQUIRED)
find_package(nlohmann_json REQUIRED)

# ---- Core library (testable) ----
add_library(smart_parking_lib
    src/Parking.cpp
    src/ParkingPlace.cpp
    src/utils.cpp
)

target_include_directories(smart_parking_lib PUBLIC
    header
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(smart_parking_lib
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
)

# ---- Main executable ----
add_executable(smart_parking main.cpp)

target_link_libraries(smart_parking smart_parking_lib)

# ---- GoogleTest ----
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_subdirectory(tests)
