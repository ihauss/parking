cmake_minimum_required(VERSION 3.10)

# Project
project(SmartParkingVision LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependencies
find_package(OpenCV REQUIRED)
find_package(nlohmann_json REQUIRED)

# Executable
add_executable(smart_parking
    main.cpp
    src/Parking.cpp
    src/ParkingPlace.cpp
    src/utils.cpp
)

# Include directories (target-scoped)
target_include_directories(smart_parking PRIVATE
    header
    ${OpenCV_INCLUDE_DIRS}
)

# Compiler warnings (Wall / Wextra / Werror equivalent)
target_compile_options(smart_parking PRIVATE
    -Wall
    -Wextra
    -Werror
    -pedantic
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(smart_parking PRIVATE -O3)
endif()


# Link libraries
target_link_libraries(smart_parking PRIVATE
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
)
