add_library(plugin1 SHARED plugin1.cpp)

target_link_libraries(plugin1 PUBLIC kernel)

install(TARGETS plugin1 DESTINATION lib/numeri-plugins)