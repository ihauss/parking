add_library(smart_parking_core_lib
    src/Parking.cpp
    src/ParkingPlace.cpp
    src/utils.cpp
    src/LightVision.cpp
    src/HeavyEstimator.cpp
    src/Aligner.cpp
    src/Renderer.cpp
)

set_target_properties(smart_parking_core_lib PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(smart_parking_core_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(smart_parking_core_lib
    PUBLIC
        ${OpenCV_LIBS}
        nlohmann_json::nlohmann_json
)
