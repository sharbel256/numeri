
FIND_PACKAGE( Boost 1.80 REQUIRED system)

find_package(OpenSSL REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_library(ui
    mainwindow.cpp 
    mainwindow.h 
    mainwindow.ui
    websocketclient.h
)

target_link_libraries(ui
    algorithm_lib
    ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES}
   Qt6::Widgets)


# Add the header file to the public interface of the library
target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
