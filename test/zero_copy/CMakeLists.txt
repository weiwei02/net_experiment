#零拷贝和用户态缓冲区对网络性能的影响 实验
add_executable(echo_server echo_server.cpp)
target_link_libraries(echo_server event)


add_executable(epoll_echo epoll_echo.cpp)
target_link_libraries(epoll_echo epoll_handler)


add_executable(splice_epoll_echo splice_epoll_echo.cpp)
target_link_libraries(splice_epoll_echo epoll_handler)


add_executable(load_client load_client.cpp)
set_target_properties(load_client PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(load_client PROPERTIES LINK_SEARCH_END_STATIC 1)
target_link_libraries(load_client -static
        event
        pthread
        )