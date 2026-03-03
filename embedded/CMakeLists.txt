add_executable(smart_parking_embedded
    main.cpp
    src/utils.cpp
)

target_include_directories(smart_parking_core_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(smart_parking_embedded
    PRIVATE smart_parking_core_lib
)