pybind11_add_module(
    smart_parking_core
    Parking.cpp
)

target_link_libraries(smart_parking_core
    PRIVATE smart_parking_core_lib
)