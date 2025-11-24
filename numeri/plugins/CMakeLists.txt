find_package(OpenSSL REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)

add_library(algorithm1 SHARED algorithm1.cpp)

target_link_libraries(algorithm1 PUBLIC 
    kernel
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto)

install(TARGETS algorithm1 DESTINATION lib/numeri-plugins)

add_library(l2ingestor SHARED l2_ingestor.cpp)
target_link_libraries(l2ingestor PUBLIC 
    kernel
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    jwt-cpp::jwt-cpp)

install(TARGETS l2ingestor DESTINATION lib/numeri-plugins)