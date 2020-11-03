//
// Created by weiwei on 2020/11/3.
//

#include <tcp/epoll_handler.h>

int main(int argc, char** argv) {
    ServerHandler serverhandler(8899);
    IOLoop::Instance()->start();
    return 0;
}