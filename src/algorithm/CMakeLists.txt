FIND_PACKAGE( Boost 1.80 REQUIRED system)

find_package(OpenSSL REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_library(algorithm_lib STATIC
  client.cpp
)

target_link_libraries(algorithm_lib ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})

# Add the header file to the public interface of the library
target_include_directories(algorithm_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
