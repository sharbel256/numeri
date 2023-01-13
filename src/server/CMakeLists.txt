add_executable(server
   server.cpp
)

TARGET_LINK_LIBRARIES( server LINK_PUBLIC ${Boost_LIBRARIES} )