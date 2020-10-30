//
// Created by weiwei on 2020/10/26.
//

#include <assert.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "tcp_proxy.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <cstdio>
#include <cstdlib>
//#include <ctype.h>
#include <unistd.h>

/**
 * 最大4MB缓存
 * */
static const int MAX_OUTPUT = 4 * 1024 * 1024;

static void _ev_read_cb(struct bufferevent *bev, void *ctx){
    auto c = (EvContext*)ctx;
    c->proxy->readcb(bev, c);
}


static void _close_on_finished_writecb(struct bufferevent *bev, void *ctx) {
    struct evbuffer *b = bufferevent_get_output(bev);

    if (evbuffer_get_length(b) == 0) {
        bufferevent_free(bev);
    }
}
static void _ev_event_cb(struct bufferevent *bev, short what, void *c){
    auto ctx = (EvContext*)c;
    ctx->proxy->eventcb(bev,what, ctx);
    auto partner =  ctx->get_target_buffer();

    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        if (what & BEV_EVENT_ERROR) {
            unsigned long err;
            if (errno)
                perror("connection error");
        }

        if (partner) {
            /* Flush all pending data */
            _ev_read_cb(bev, ctx);

            if (evbuffer_get_length(
                    bufferevent_get_output(partner))) {
                /* We still have to flush data from the other
                 * side, but when that's done, close the other
                 * side. */
                bufferevent_setcb(partner,
                                  NULL, _close_on_finished_writecb,
                                  _ev_event_cb, ctx);
                bufferevent_disable(partner, EV_READ);
            } else {
                /* We have nothing left to say to the other
                 * side; close it. */
                bufferevent_free(partner);
            }
        }
        ctx->proxy
        bufferevent_free(bev);
    }
}

static void _ev_write_cb(struct bufferevent *bev, void *c){
    auto ctx = (EvContext*)c;
    ctx->proxy->writecb(bev, ctx);

}

static void _drained_writecb(struct bufferevent *bev, void *ctx) {
    struct bufferevent *partner = (struct bufferevent *) ctx;


}

TcpProxy::TcpProxy(std::shared_ptr<IoContext> &ioContext): ioContext(ioContext){
    this->accept_source_connection = [this] (int fd, sockaddr *a, int slen, void *p){ return default_accept_cb(fd, a, slen, p); };
    this->readcb = [this](bufferevent *bev, EvContext* ctx) { default_read_cb(bev,ctx); };
    this->writecb = [this](bufferevent *bev, EvContext* ctx) { default_write_cb(bev,ctx); };
    this->eventcb = [this](bufferevent *bev, short what, EvContext* ctx) { default_event_cb(bev, what,ctx); };
}
TcpProxy::~TcpProxy() {
    printf("开始代理连接资源回收");
    bufferevent_free(b_out);
    bufferevent_free(b_in);
}

void TcpProxy::default_read_cb(bufferevent *bev, EvContext* ctx) {
    auto partner = ctx->get_target_buffer();
    evbuffer *src, *dst;
    size_t len;
    src = bufferevent_get_input(bev);
    len = evbuffer_get_length(src);

    // 如果对端不存在，则排空当前数据
    if (!partner) {
        evbuffer_drain(src, len);
        return;
    }

    dst = bufferevent_get_output(partner);
    evbuffer_add_buffer(dst, src);

    if (evbuffer_get_length(dst) >= MAX_OUTPUT) {
        /* We're giving the other side data faster than it can
         * pass it on.  Stop reading here until we have drained the
         * other side to MAX_OUTPUT/2 bytes. */
        bufferevent_setcb(partner, _ev_read_cb, _ev_write_cb,
                          _ev_event_cb, ctx->get_partner());
        bufferevent_setwatermark(partner, EV_WRITE, MAX_OUTPUT / 2,
                                 MAX_OUTPUT);
        bufferevent_disable(bev, EV_READ);

    }
}

void TcpProxy::default_write_cb(bufferevent *bev, EvContext* ctx) {
    auto partner = ctx->get_target_buffer();
    /* We were choking the other side until we drained our outbuf a bit.
     * Now it seems drained. */
    bufferevent_setcb(bev, _ev_read_cb, NULL, _ev_event_cb, partner);
    bufferevent_setwatermark(bev, EV_WRITE, 0, 0);
    if (partner)
        bufferevent_enable(partner, EV_READ);
}

void TcpProxy::default_event_cb(bufferevent *bev, short what, EvContext* ctx) {

}

bool TcpProxy::default_accept_cb(int fd, sockaddr *a, int slen, void *p) {
    // io 进出使用不同的调度
    b_in = bufferevent_socket_new(ioContext->io_base(), fd,
                                  BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
    b_out = bufferevent_socket_new(ioContext->io_base(), -1,
                                   BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
    assert(b_in && b_out);

    proxyConnection = std::make_unique<TcpProxyConnection>((sockaddr_storage *)(a), slen, fd);
    // 获取原始目标地址成功，可以启用透明代理
    if (!proxyConnection->set_origin_dest_addr()){
        return false;
    }
    if (bufferevent_socket_connect(b_out,
                                   (struct sockaddr *) &proxyConnection->dst_addr, proxyConnection->dst_len) < 0) {
        perror("bufferevent_socket_connect error");
        return false;
    }

    out = std::make_unique<EvContext>(1, this);
    in = std::make_unique<EvContext>(0, this);

    bufferevent_setcb(b_in, _ev_read_cb, NULL, _ev_event_cb, out.get());
    bufferevent_setcb(b_out, _ev_read_cb, NULL, _ev_event_cb, in.get());
    bufferevent_enable(b_in, EV_READ | EV_WRITE);
    bufferevent_enable(b_out, EV_READ | EV_WRITE);
}


void TcpProxyHolder::append(std::unique_ptr<TcpProxy> && proxy) {
//    proxy->code = index++;
    proxy->holder = shared_from_this();
    hmap.insert(std::pair((int64_t)proxy.get(), std::move(proxy)));
}

void TcpProxyHolder::remove(int64_t i) {
    hmap.erase(i);
}

TcpProxyConnection::TcpProxyConnection(sockaddr_storage *src_addr, int src_len, int in_fd):src_len(src_len),src_addr(*src_addr), in_fd(in_fd) {

}

bool TcpProxyConnection::set_origin_dest_addr() {
// get origin dest ，it mean use iptables
    auto addr = (sockaddr_in *) &origin_dst_addr;
    if( getsockopt(in_fd, SOL_SOCKET, IP_ORIGDSTADDR, addr, &origin_len) == 0){
        dst_addr = origin_dst_addr;
        dst_len = origin_len;
        char ip[20];
        printf("transparent_proxy : origin dest connection[%s:%d]\n", inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip)), ntohs(addr->sin_port));
    }
}

void TcpProxyConnection::set_transparent_proxy() {

}


