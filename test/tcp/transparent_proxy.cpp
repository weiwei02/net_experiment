// 单线程透明代理程序
// Created by weiwei on 2020/10/29.
//

#include <memory>
#include <tcp/tcp_proxy.h>
#include "common/concurrency/IoContext.h"
#include <map>

int main(int argc, char **argv){
    auto hold = std::make_shared<TcpProxyHolder>();
    auto ioContext = std::make_shared<IoContext>(1);
    ioContext->listen("0.0.0.0:49913", [&ioContext,hold](evutil_socket_t fd,
                             struct sockaddr *a, int slen, void *){
        auto proxy = std::make_unique<TcpProxy>(ioContext);

        // 接收新的外部连接
        if (proxy->accept_source_connection(fd, a, slen, nullptr)){
            hold->append(std::move(proxy));
        }

        return true;
    });
}