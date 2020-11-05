#include <tcp/epoll_handler.h>
#include <assert.h>

int main(int argc, char** argv) {
    assert(argc >= 2);
    ServerHandler serverhandler(atoi(argv[1]));
    IOLoop::Instance()->start();
    return 0;
}