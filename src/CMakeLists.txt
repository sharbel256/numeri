cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(dynalo VERSION 2022.0.0 LANGUAGES CXX C)
message(STATUS "${CMAKE_PROJECT_NAME} version: ${PROJECT_VERSION}")


add_executable(server
   server.cpp
)