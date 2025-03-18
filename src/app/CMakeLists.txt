add_executable(main
    main.cpp
    trading.h
    readers.h
)

add_library(orderbook
    orderbook.h
    orderbook.cpp
)

target_link_libraries(main 
    service_lib
    orderbook
)