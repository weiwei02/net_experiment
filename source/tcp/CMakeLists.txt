
add_executable(le_proxy le_proxy.cpp ../../test/tcp/transparent_proxy.cpp )
target_link_libraries(le_proxy event)

add_library(tcp_proxy tcp_proxy.h tcp_proxy.cpp)
target_link_libraries(tcp_proxy concurrency)

add_library(t_proxy STATIC TransparentTcpProxy.cpp TransparentTcpProxy.h)
target_link_libraries(t_proxy concurrency)

add_library(epoll_handler STATIC epoll_handler.cpp epoll_handler.h)