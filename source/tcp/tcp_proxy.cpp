//
// Created by weiwei on 2020/10/26.
//

#include "tcp_proxy.h"

TcpProxy::~TcpProxy() {
    bufferevent_free(b_out);
    bufferevent_free(b_in);
}

void TcpProxy::default_read_cb(bufferevent *bev, void *ctx) {

}

void TcpProxy::default_write_cb(bufferevent *bev, void *ctx) {

}

void TcpProxy::default_event_cb(bufferevent *bev, short what, void *ctx) {

}

void TcpProxy::default_accept_cb(int fd, sockaddr *a, int slen, void *p) {

}
