cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(numeri VERSION 2023.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(OPENSSL_ROOT_DIR "/opt/homebrew/Cellar/openssl@1.1/1.1.1t/include")
set(OPENSSL_CRYPTO_LIBRARY "/opt/homebrew/Cellar/openssl@1.1/1.1.1t/lib/libcrypto.dylib")
set(OPENSSL_SSL_LIBRARY "/opt/homebrew/Cellar/openssl@1.1/1.1.1t/lib/libssl.dylib")

message(STATUS "${CMAKE_PROJECT_NAME} version: ${PROJECT_VERSION}")

find_package(Qt6 CONFIG COMPONENTS Widgets REQUIRED)

add_subdirectory(algorithm)
add_subdirectory(app)



