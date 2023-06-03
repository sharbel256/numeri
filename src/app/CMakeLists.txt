find_package(Qt6 CONFIG COMPONENTS Widgets REQUIRED)

add_subdirectory(ui)

add_executable(main
    main.cpp
)

target_link_libraries(main 
    Qt6::Widgets
    algorithm_lib
    ui
)