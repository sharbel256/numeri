FIND_PACKAGE(Boost 1.80 REQUIRED system)
find_package(nlohmann_json CONFIG REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

add_library(ui
    mainwindow.cpp 
    mainwindow.h 
    mainwindow.ui
)

target_link_libraries(ui
    nlohmann_json::nlohmann_json
    ${Boost_LIBRARIES}
    Qt6::Widgets
    service_lib
)

target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
