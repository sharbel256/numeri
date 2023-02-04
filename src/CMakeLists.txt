cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(numeri VERSION 2023.1.0 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "${CMAKE_PROJECT_NAME} version: ${PROJECT_VERSION}")

find_package(Boost 1.81 COMPONENTS program_options REQUIRED)
find_package(CURL REQUIRED)

include_directories(${CURL_INCLUDE_DIR})

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

add_subdirectory(server)