cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(numeri VERSION 2023.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

message(STATUS "${CMAKE_PROJECT_NAME} version: ${PROJECT_VERSION}")

find_package(Qt6 CONFIG COMPONENTS Widgets REQUIRED)

add_subdirectory(service)
add_subdirectory(app)



