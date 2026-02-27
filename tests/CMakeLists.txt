include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(core_tests
    test_utils.cpp
    test_parking.cpp
    test_parking_place.cpp
)

target_link_libraries(core_tests
    PRIVATE
        smart_parking_core_lib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(core_tests)

