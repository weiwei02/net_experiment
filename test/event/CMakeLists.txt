
add_executable(event_test echo_server.cpp splice_echo.cpp splice_epoll_echo.cpp)
target_link_libraries(event_test event)


add_executable(splice_echo splice_echo.cpp)