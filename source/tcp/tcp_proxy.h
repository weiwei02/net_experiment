//
// Created by weiwei on 2020/10/26.
//

#ifndef NET_EXP_TCP_PROXY_H
#define NET_EXP_TCP_PROXY_H

#include <sys/socket.h>
#include <event.h>

#include <memory>
#include <functional>
#include <map>
#include "../common/concurrency/IoContext.h"

/**
 * tcp 代理代理链接描述信息
 * */
class TcpProxyConnection{
public:
    sockaddr_storage src_addr;
    sockaddr_storage origin_dst_addr;
    sockaddr_storage dst_addr;
    /**
     * 下游和上游文件描述符
     * */
    evutil_socket_t in_fd, out_fd;

    /**
     * 构建原始透明代理对象
     * 这个构造函数会求出原始目标地址
     * */
    TcpProxyConnection(
            sockaddr_storage src_addr,
    sockaddr_storage origin_dst_addr,
    sockaddr_storage dst_addr,
    evutil_socket_t in_fd):src_addr(src_addr), in_fd(in_fd){};

    void get_origin_dest_addr();

    /**
     * 设置透明代理
     * */
    void set_transparent_proxy();
};

class TcpProxy{

public:
    std::unique_ptr<TcpProxyConnection> proxyConnection;
    bufferevent *b_out, *b_in;
    // 适用的分发器
    std::shared_ptr<event_base> base;
    std::shared_ptr<IoContext> ioContext;
    // 对象唯一编号
//    int64_t code;

    TcpProxy(std::shared_ptr<IoContext>& ioContext);
    ~TcpProxy();

    /**
     * 与目标段建立网络链接
     * */


    /**
     * 设置默认相关回调函数
     * */
    IoContext::accept_cb accept_source_connection;
    bufferevent_data_cb readcb;
    bufferevent_data_cb writecb;
    bufferevent_event_cb eventcb;

private:
    /**
     * 默认的回调函数
     * */
    void default_read_cb(bufferevent *bev, void *ctx);
    void default_write_cb(bufferevent *bev, void *ctx);
    void default_event_cb(bufferevent *bev, short what, void *ctx);
    void default_accept_cb(evutil_socket_t fd, sockaddr *a, int slen, void *p);
};

class TcpProxyHolder{
private:
    std::map<int64_t , std::unique_ptr<TcpProxy>> hmap;
    int64_t index = 0;
public:
    void append(std::unique_ptr<TcpProxy>&&);
    // todo 注意线程安全
    void remove(int64_t i);
};
#endif //NET_EXP_TCP_PROXY_H
