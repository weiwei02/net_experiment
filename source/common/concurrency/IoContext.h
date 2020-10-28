//
// Created by weiwei on 2020/10/28.
//

#ifndef NET_EXP_IOCONTEXT_H
#define NET_EXP_IOCONTEXT_H
#include <event.h>

class IoContext {
private:
    // thread count
    int threads;
    // 当前选择的io索引
    int chose_io_index;
    event_base** base_array;
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
};


#endif //NET_EXP_IOCONTEXT_H
