add_executable(main
   main.cpp
)

TARGET_LINK_LIBRARIES( main LINK_PUBLIC ${Boost_LIBRARIES} ${CURL_LIBRARIES})

