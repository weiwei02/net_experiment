//
// Created by weiwei on 2020/10/30.
//

#ifndef NET_EXP_TRANSPARENTTCPPROXY_H
#define NET_EXP_TRANSPARENTTCPPROXY_H


#include <common/concurrency/IoContext.h>

class TransparentTcpProxy{
    std::shared_ptr<IoContext> ioContext;
    event_base* base;
public:

    TransparentTcpProxy* partner;
    sockaddr_storage addr;
    socklen_t src_len;
    evutil_socket_t fd;

    bufferevent *buffer;
    // 0:in 1:out
    int type = 0;

    TransparentTcpProxy(std::shared_ptr<IoContext>& ioContext);
    ~TransparentTcpProxy();
    bool default_accept_cb(evutil_socket_t fd, sockaddr *a, int slen, void *p);

    TransparentTcpProxy* init_transparent_partner(TransparentTcpProxy* partner);

    bool connect_to_upstream();

    void start();
};


#endif //NET_EXP_TRANSPARENTTCPPROXY_H
