cmake_minimum_required(VERSION 3.15)
project(smart_parking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# Dépendances globales
find_package(OpenCV REQUIRED)
find_package(nlohmann_json REQUIRED)

# Sous-modules
option(BUILD_API "Build Python API bindings" ON)
option(BUILD_EMBEDDED "Build embedded executable" ON)

add_subdirectory(extern/pybind11)
add_subdirectory(core)

if (BUILD_EMBEDDED)
    add_subdirectory(embedded)
endif()

if (BUILD_API)
    add_subdirectory(bindings)
endif()

# tests seulement en dev
option(BUILD_TESTS "Build tests" OFF)
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()
