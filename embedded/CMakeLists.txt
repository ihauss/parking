add_executable(smart_parking_embedded
    main.cpp
)

target_link_libraries(smart_parking_embedded
    PRIVATE smart_parking_core
)