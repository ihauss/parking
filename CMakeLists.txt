cmake_minimum_required(VERSION 3.10)

project(Week5)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(nlohmann_json REQUIRED)

include_directories(main
    ${OpenCV_INCLUDE_DIRS}
    header
)

add_executable(main
    main.cpp
    src/Parking.cpp
    src/ParkingPlace.cpp
    src/utils.cpp
)

target_link_libraries(main
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
)
