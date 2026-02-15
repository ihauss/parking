cmake_minimum_required(VERSION 3.15)
project(smart_parking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# Dépendances globales
find_package(OpenCV REQUIRED)
find_package(nlohmann_json REQUIRED)

# Sous-modules
add_subdirectory(core)
add_subdirectory(embedded)
add_subdirectory(tests)
