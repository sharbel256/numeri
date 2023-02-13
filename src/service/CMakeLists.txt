set (CMAKE_CXX_STANDARD 11)

FIND_PACKAGE( Boost 1.80 REQUIRED system)
find_package(CURL REQUIRED)
find_package(Poco REQUIRED Foundation Net)
include_directories(${CURL_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${Poco_INCLUDE_DIRS})

add_executable(service
  service.cpp
)

target_link_libraries(service ${CURL_LIBRARIES} ${Boost_LIBRARIES} ${Poco_LIBRARIES})