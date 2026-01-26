add_executable(run_tests
    test_utils.cpp
    test_parking.cpp
    test_parking_place.cpp
)

target_link_libraries(run_tests
    gtest
    gtest_main
    smart_parking_lib
)

include(GoogleTest)
gtest_discover_tests(run_tests)
