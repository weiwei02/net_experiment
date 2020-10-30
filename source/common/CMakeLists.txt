add_library(concurrency STATIC concurrency/IoContext.cpp concurrency/IoContext.h)
target_link_libraries(concurrency event)