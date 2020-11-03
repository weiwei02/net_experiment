
add_executable(echo_server echo_server.cpp)
target_link_libraries(echo_server event)


add_executable(splice_echo splice_echo.cpp)

add_executable(epoll_echo epoll_echo.cpp)
target_link_libraries(epoll_echo epoll_handler)


add_executable(splice_epoll_echo splice_epoll_echo.cpp)
target_link_libraries(splice_epoll_echo epoll_handler)