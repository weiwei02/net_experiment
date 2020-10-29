//
// Created by weiwei on 2020/10/28.
//

#ifndef NET_EXP_IOCONTEXT_H
#define NET_EXP_IOCONTEXT_H
#include <event.h>
#include <string>
#include <memory>
#include <functional>

class IoContext :public std::enable_shared_from_this<IoContext>{
public:
    // 如何接受新链接
    using accept_cb = std::function<bool(evutil_socket_t fd, sockaddr *a, int slen, void *p)> ;
private:
    // thread count
    int threads;
    // 当前选择的io索引
    int chose_io_index;
    event_base** base_array;
    // 监听地址
    sockaddr_storage listen_on_addr;
    std::string address;

public:
    IoContext(int threads);

    ~IoContext();
    /**
     * 获取主event，这个eventbase是位于主线程的
     * */
    event_base* main_base();

//    随机获取一个io event_base
    event_base* io_base();

    /**
     * 阻塞当前线程，开始轮循分发事件
     * */
    void dispatch();

    /**
     * 释放所有控制器
     * */
    void free();

    int listen(const std::string& address, accept_cb cb);

    /**
     * 接受链接回调函数
     * */
    accept_cb acceptCb;
};


#endif //NET_EXP_IOCONTEXT_H
