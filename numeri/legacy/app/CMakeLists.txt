find_package(jwt-cpp CONFIG REQUIRED)

add_executable(main
    main.cpp
    trading.h
    readers.h
    constants.h
    orderbuilder.cpp
)

add_library(orderbook SHARED
    orderbook.h
    orderbook.cpp
)

target_link_libraries(main 
    service_lib
    orderbook
    jwt-cpp::jwt-cpp
)