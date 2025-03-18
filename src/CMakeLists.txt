cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(numeri VERSION 2023.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(OPENSSL_ROOT_DIR "/opt/homebrew/Cellar/openssl@1.1/1.1.1t")
set(OPENSSL_CRYPTO_LIBRARY "/opt/homebrew/Cellar/openssl@1.1/1.1.1t/lib/libcrypto.dylib")
set(OPENSSL_SSL_LIBRARY "/opt/homebrew/Cellar/openssl@1.1/1.1.1t/lib/libssl.dylib")

message(STATUS "${CMAKE_PROJECT_NAME} version: ${PROJECT_VERSION}")



# Include directories for headers
include_directories(
    ${CMAKE_SOURCE_DIR}/app
    ${CMAKE_SOURCE_DIR}/service
)

# Add subdirectories
add_subdirectory(app)
add_subdirectory(service)
# add_subdirectory(${CMAKE_SOURCE_DIR}/../import/googletest)  # Include Google Test
add_subdirectory(tests)  # Add the tests directory

add_subdirectory(${CMAKE_SOURCE_DIR}/../import/googletest ${CMAKE_BINARY_DIR}/googletest-build)
