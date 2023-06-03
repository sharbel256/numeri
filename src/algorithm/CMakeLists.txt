set (CMAKE_CXX_STANDARD 11)

set(OPENSSL_ROOT_DIR "/opt/homebrew/Cellar/openssl@1.1/1.1.1t/include")
set(OPENSSL_CRYPTO_LIBRARY "/opt/homebrew/Cellar/openssl@1.1/1.1.1t/lib/libcrypto.dylib")
set(OPENSSL_SSL_LIBRARY "/opt/homebrew/Cellar/openssl@1.1/1.1.1t/lib/libssl.dylib")


FIND_PACKAGE( Boost 1.80 REQUIRED system)

find_package(OpenSSL REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_library(algorithm_lib STATIC
  client.cpp
)

target_link_libraries(algorithm_lib ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})

# Add the header file to the public interface of the library
target_include_directories(algorithm_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
