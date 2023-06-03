FIND_PACKAGE( Boost 1.80 REQUIRED system)

include_directories(${Boost_INCLUDE_DIR})

add_library(ui
    mainwindow.cpp 
    mainwindow.h 
    mainwindow.ui
    websocketclient.h
)

target_link_libraries(ui
    ${Boost_LIBRARIES}
    Qt6::Widgets
    algorithm_lib
)


target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
