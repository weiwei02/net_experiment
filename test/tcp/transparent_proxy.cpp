// 单线程透明代理程序
// Created by weiwei on 2020/10/29.
//

#include <memory>
#include <tcp/TransparentTcpProxy.h>
#include "common/concurrency/IoContext.h"
#include <map>

int main(int argc, char **argv){
    auto ioContext = std::make_shared<IoContext>(1);
    auto serverStr = "0.0.0.0:5001";
    ioContext->listen(serverStr, [&ioContext](evutil_socket_t fd,
                             struct sockaddr *a, int slen, void *){
        auto proxy = new TransparentTcpProxy(ioContext);
        if (!proxy->default_accept_cb(fd, a, slen, nullptr)){
            delete proxy->partner;
            delete proxy;
            return false;
        }
        return true;
    });
    printf("server listen %s\n", serverStr);
    ioContext->dispatch();
}