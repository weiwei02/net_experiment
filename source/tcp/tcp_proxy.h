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
class TcpProxy;
class EvContext;
/**
 * tcp 代理代理链接描述信息
 * */
class TcpProxyConnection{
public:
    sockaddr_storage src_addr;
    sockaddr_storage origin_dst_addr;
    sockaddr_storage dst_addr;
    socklen_t src_len, origin_len,dst_len;
    /**
     * 下游和上游文件描述符
     * */
    evutil_socket_t in_fd, out_fd;

    /**
     * 构建原始透明代理对象
     * 这个构造函数会求出原始目标地址
     * */
    TcpProxyConnection(
            sockaddr_storage * src_addr,
            int src_len,
    evutil_socket_t in_fd);

    bool set_origin_dest_addr();

    /**
     * 设置透明代理
     * */
    void set_transparent_proxy();
};
class TcpProxyHolder;
class TcpProxy{

public:
    std::unique_ptr<TcpProxyConnection> proxyConnection;
    bufferevent *b_out, *b_in;
    // 适用的分发器
    std::shared_ptr<event_base> base;
    std::shared_ptr<IoContext> ioContext;
    std::shared_ptr<TcpProxyHolder> holder;
    std::unique_ptr<EvContext> in, out;
    // 对象唯一编号
//    int64_t code;

/**
 * 默认情况下使用tcp代理的默认方法
 * */
    TcpProxy(std::shared_ptr<IoContext>& ioContext);
    ~TcpProxy();

    /**
     * 与目标段建立网络链接
     * */


    /**
     * 设置默认相关回调函数
     * */
    IoContext::accept_cb accept_source_connection;
    std::function<void(struct bufferevent *bev, EvContext* ctx)> readcb;
    std::function<void(struct bufferevent *bev, EvContext* ctx)> writecb;
    std::function<void(struct bufferevent *bev, short what, EvContext* ctx)> eventcb;

private:
    /**
     * 默认的回调函数
     * */
    void default_read_cb(bufferevent *bev, EvContext* ctx);
    void default_write_cb(bufferevent *bev, EvContext* ctx);
    void default_event_cb(bufferevent *bev, short what, EvContext* ctx);
    bool default_accept_cb(evutil_socket_t fd, sockaddr *a, int slen, void *p);
};



class TcpProxyHolder : public std::enable_shared_from_this<TcpProxyHolder>{
private:
    std::map<int64_t , std::unique_ptr<TcpProxy>> hmap;
    int64_t index = 0;
public:
    void append(std::unique_ptr<TcpProxy>&&);
    // todo 注意线程安全
    void remove(int64_t i);
};

class EvContext{

public:
    EvContext(const int target, TcpProxy* proxy):target(target), proxy(proxy){}
// 0: in向上游发数据，1:out 向下游发数据
    const int target;;
    TcpProxy* proxy;

    bufferevent* get_target_buffer() {
        if (proxy){
            if (target){
                return proxy->b_out;
            }
            return proxy->b_in;
        }
        return nullptr;
    }

    EvContext* get_partner(){
        if (proxy){
            if (target){
                return proxy->out.get();
            }
            return proxy->in.get();
        }
        return nullptr;
    }
};
#endif //NET_EXP_TCP_PROXY_H
