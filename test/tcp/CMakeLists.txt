
add_executable(transparent_proxy transparent_proxy.cpp)
target_link_libraries(transparent_proxy t_proxy)

add_executable(load_client load_client.cpp)
set_target_properties(load_client PROPERTIES LINK_SEARCH_START_STATIC 1)
set_target_properties(load_client PROPERTIES LINK_SEARCH_END_STATIC 1)
target_link_libraries(load_client -static
        event
        pthread
        )