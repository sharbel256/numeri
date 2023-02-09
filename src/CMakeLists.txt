cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(numeri VERSION 2023.1.0 LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "${CMAKE_PROJECT_NAME} version: ${PROJECT_VERSION}")


find_package(CURL REQUIRED)
find_package(Poco REQUIRED Foundation Net)


include_directories(${CURL_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${Poco_INCLUDE_DIRS})


add_subdirectory(service)


add_executable(main
   main.cpp
)

# TARGET_LINK_LIBRARIES( main LINK_PUBLIC 
# Poco::Foundation 
# Poco::Net 
# ${Boost_LIBRARIES} 
# ${CURL_LIBRARIES} )