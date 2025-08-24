cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(numeri VERSION 2025.1.1 LANGUAGES CXX)

set(NUMERI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(NUMERI_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_FIND_PACKAGE_NO_SYSTEM_PACKAGES ON)
set(CMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY OFF)
set(CMAKE_FIND_USE_PACKAGE_REGISTRY OFF)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ language version to use (default is 17)")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_TOOLS "Enable building tools" ON)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_COMPILER_WARNINGS "Build with compiler warnings" OFF)
option(ENABLE_WERROR "Convert compiler warnings to errors" OFF)

set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "VCPKG toolchain file")

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

message(STATUS "${CMAKE_PROJECT_NAME} version: ${PROJECT_VERSION}")
message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")

# Colorize output
include(CheckCXXCompilerFlag)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  check_cxx_compiler_flag("-fdiagnostics-color=auto" HAS_COLOR_FLAG)
  if(HAS_COLOR_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=auto")
  endif()
endif()

add_subdirectory(src)
add_subdirectory(test)

find_package(Git QUIET)
if(GIT_FOUND)
  add_custom_target(submodule-update
    COMMAND ${GIT_EXECUTABLE} submodule sync --recursive
    COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
    COMMAND ${GIT_EXECUTABLE} submodule foreach --recursive git lfs pull
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Updating Git submodules"
  )
endif()

add_custom_target(format
  COMMAND ${CMAKE_SOURCE_DIR}/.format.sh
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Running format script"
)