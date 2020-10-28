//
// Created by weiwei on 2020/10/26.
//

#ifndef NET_EXP_TCP_PROXY_H
#define NET_EXP_TCP_PROXY_H

#include <sys/socket.h>
#include <event.h>

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
    TcpProxyConnection proxyConnection;

};

#endif //NET_EXP_TCP_PROXY_H
