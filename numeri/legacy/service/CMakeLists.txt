FIND_PACKAGE( Boost 1.80 REQUIRED system)

include_directories(${Boost_INCLUDE_DIR})

add_library(service_lib STATIC
  websocketclient.h
  websocketclient.cpp
  httpclient.h
  httpclient.cpp
  logging.h
)

target_link_libraries(service_lib ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})

target_include_directories(service_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
