//
// Created by weiwei on 2020/11/3.
//

#ifndef NET_EXP_EPOLL_HANDLER_H
#define NET_EXP_EPOLL_HANDLER_H


#include <sys/epoll.h>
#include <iostream>
#include <unordered_map>


#define MAX_PENDING 1024
#define BUFFER_SIZE 1024
class Handler {
public:
    virtual ~Handler() {}
    virtual int handle(epoll_event e) = 0;
};
/**
 * epoll 事件轮询
 */
class IOLoop {
public:
    static IOLoop *Instance();

    ~IOLoop();

    void start();

    void addHandler(int fd, Handler* handler, unsigned int events);

    void modifyHandler(int fd, unsigned int events);

    void removeHandler(int fd);

private:
    IOLoop();

private:
    int epfd_;
    std::unordered_map<int, Handler*> handlers_;
};


class EchoHandler : public Handler{
public:
    EchoHandler();
    virtual int handle(epoll_event e) override;
private:
    int received = 0;
    char buffer[BUFFER_SIZE];
};

class ServerHandler : public Handler{
public:
    ServerHandler(int port);
    virtual int handle(epoll_event e) override;
private:
    void setnonblocking(int fd);
};

class SpliceEchoHandler : public Handler{
public:
    SpliceEchoHandler();
    ~SpliceEchoHandler();
    virtual int handle(epoll_event e) override;
private:
    int received = 0;
    int pipefd[2];
};

#endif //NET_EXP_EPOLL_HANDLER_H
