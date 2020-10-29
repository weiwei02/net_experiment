
add_executable(le_proxy le_proxy.cpp ../../test/tcp/transparent_proxy.cpp)
target_link_libraries(le_proxy event)

add_library(tcp_proxy tcp_proxy.h tcp_proxy.cpp)
target_link_libraries(tcp_proxy concurrency)