//
// Created by weiwei on 2020/10/30.
//

#include <arpa/inet.h>
#include "TransparentTcpProxy.h"

//输出缓冲区最大空间
static const int MAX_OUTPUT = 4 * 1024 * 1024;

static void _drained_writecb(struct bufferevent *bev, void *ctx);

static void _eventcb(struct bufferevent *bev, short what, void *ctx);

static void _readcb(struct bufferevent *bev, void *ctx) {
    auto partner = static_cast<TransparentTcpProxy *>(ctx);
    struct evbuffer *src, *dst;
    size_t len;
    src = bufferevent_get_input(bev);
    len = evbuffer_get_length(src);

    // 对端为空，应排空缓冲区后释放连接
    if (!partner->buffer) {
        evbuffer_drain(src, len);
        return;
    }
    dst = bufferevent_get_output(partner->buffer);
    evbuffer_add_buffer(dst, src);

    // 输出缓冲区满
    if (evbuffer_get_length(dst) >= MAX_OUTPUT) {
        /* We're giving the other side data faster than it can
         * pass it on.  Stop reading here until we have drained the
         * other side to MAX_OUTPUT/2 bytes. */
        bufferevent_setcb(partner->buffer, _readcb, _drained_writecb,
                          _eventcb, partner->partner);
        bufferevent_setwatermark(partner->buffer, EV_WRITE, MAX_OUTPUT / 2,
                                 MAX_OUTPUT);
        bufferevent_disable(bev, EV_READ);
    }
}


//当前对端输出缓冲区已经至少有一半空闲，此时允许将源输入缓冲区的数据做读取
static void _drained_writecb(struct bufferevent *bev, void *ctx) {
    auto partner = static_cast<TransparentTcpProxy *>(ctx);

    /* We were choking the other side until we drained our outbuf a bit.
     * Now it seems drained. */
    bufferevent_setcb(bev, _readcb, NULL, _eventcb, partner);
    bufferevent_setwatermark(bev, EV_WRITE, 0, 0);
    if (partner->buffer)
        bufferevent_enable(partner->buffer, EV_READ);
}


// close connection when finnish write
static void __close_on_finished_writecb(struct bufferevent *bev, void *ctx) {
    struct evbuffer *b = bufferevent_get_output(bev);

    if (evbuffer_get_length(b) == 0) {
        bufferevent_free(bev);
    }
}

static void
_eventcb(struct bufferevent *bev, short what, void *ctx) {
    auto partner = static_cast<TransparentTcpProxy *>(ctx);

    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        if (what & BEV_EVENT_ERROR) {
            if (errno)
                perror("connection error");
        }

        if (partner) {
            /* Flush all pending data */
            _readcb(bev, ctx);

            if (evbuffer_get_length(
                    bufferevent_get_output(partner->buffer))) {
                /* We still have to flush data from the other
                 * side, but when that's done, close the other
                 * side. */
                bufferevent_setcb(partner->buffer,
                                  NULL, __close_on_finished_writecb,
                                  _eventcb, NULL);
                bufferevent_disable(partner->buffer, EV_READ);
            } else {
                /* We have nothing left to say to the other
                 * side; close it. */
                bufferevent_free(partner->buffer);
            }
        }
        bufferevent_free(bev);
    }
}

TransparentTcpProxy::TransparentTcpProxy(std::shared_ptr<IoContext> &ioContext):ioContext(ioContext) {
    base = ioContext->io_base();
}

bool TransparentTcpProxy::default_accept_cb(int fd, sockaddr *a, int slen, void *p) {
    this->fd = fd;
    this->addr = *(sockaddr_storage*)a;
    this->src_len = slen;
    // io 进出使用不同的调度
    buffer = bufferevent_socket_new(base, fd,BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
    if (!init_transparent_partner(this)){
        return false;
    }
    if (!connect_to_upstream()){
        return false;
    }
    start();
    partner->start();
    return false;
}

TransparentTcpProxy* TransparentTcpProxy::init_transparent_partner(TransparentTcpProxy *p) {
    partner = new TransparentTcpProxy(p->ioContext);
    partner->partner = p;
    partner->type = 0;
    partner->base = ioContext->io_base();


    // get origin dest ，it mean use iptables
    auto addr2 = (sockaddr_in *) &partner->addr;
    if( getsockopt(fd, SOL_SOCKET, IP_ORIGDSTADDR, addr2, &partner->src_len) == 0){
        char ip[20];
        printf("transparent_proxy : origin dest connection[%s:%d]\n", inet_ntop(AF_INET, &addr2->sin_addr, ip, sizeof(ip)), ntohs(addr2->sin_port));
    }


    return partner;
}

bool TransparentTcpProxy::connect_to_upstream() {
    buffer = bufferevent_socket_new(base, -1,
                                             BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
    if (bufferevent_socket_connect(partner->buffer,
                                   (struct sockaddr *) &addr, src_len) < 0) {
        perror("bufferevent_socket_connect error");
        return false;
    }
    fd = bufferevent_getfd(buffer);

    return true;
}

void TransparentTcpProxy::start() {
    bufferevent_setcb(buffer, _readcb, NULL,_eventcb , partner);
}


// 析构函数，关闭网络连接
TransparentTcpProxy::~TransparentTcpProxy() {
    bufferevent_free(buffer);
}
