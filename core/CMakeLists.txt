add_library(smart_parking_core
    src/Parking.cpp
    src/ParkingPlace.cpp
    src/utils.cpp
    src/LightVision.cpp
    src/HeavyEstimator.cpp
    src/Aligner.cpp
    src/Renderer.cpp
)

target_include_directories(smart_parking_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(smart_parking_core
    PUBLIC
        ${OpenCV_LIBS}
        nlohmann_json::nlohmann_json
)
