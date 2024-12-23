find_package(jwt-cpp CONFIG REQUIRED)


add_executable(main
    main.cpp
    trading.h
    readers.h
    constants.h
)

add_library(orderbook
    orderbook.h
    orderbook.cpp
)

target_link_libraries(main 
    service_lib
    orderbook
    jwt-cpp::jwt-cpp
)