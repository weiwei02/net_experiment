//
// Created by weiwei on 2020/11/3.
//

#include <sys/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <bits/fcntl-linux.h>
#include <fcntl.h>
#include <zconf.h>
#include "epoll_handler.h"
static int _data_len = 4 * 1024 * 1024;
IOLoop * IOLoop::Instance() {
    static IOLoop instance;
    return &instance;
}

IOLoop::~IOLoop() {
    for (auto it : handlers_) {
        delete it.second;
    }
}

void IOLoop::start()
{
    const int MAX_EVENTS = 10;
    epoll_event events1[MAX_EVENTS];
    while (1)
    {
        // -1 只没有事件一直阻塞
        int nfds = epoll_wait(epfd_, events1, MAX_EVENTS, -1/*Timeout*/);
        for (int i = 0; i < nfds; ++i) {
            int fd = events1[i].data.fd;
            Handler* handler = handlers_[fd];
            handler->handle(events1[i]);
        }
    }
}

void IOLoop::addHandler(int fd, Handler *handler, unsigned int events)
{
    handlers_[fd] = handler;
    epoll_event e;
    e.data.fd = fd;
    e.events = events;

    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &e) < 0) {
        std::cout << "Failed to insert handler to epoll" << std::endl;
    }
}

void IOLoop::modifyHandler(int fd, unsigned int events)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &event);
}

void IOLoop::removeHandler(int fd)
{
    Handler* handler = handlers_[fd];
    handlers_.erase(fd);
    delete handler;
    //将fd从epoll堆删除
    epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
}

IOLoop::IOLoop()
{
    epfd_ = epoll_create1(0);  //flag=0 等价于epll_craete
    if (epfd_ < 0) {
        std::cout << "Failed to create epoll" << std::endl;
        exit(1);
    }
}

EchoHandler::EchoHandler() {}

int EchoHandler::handle(epoll_event e) {
    int fd = e.data.fd;
    if (e.events & EPOLLHUP) {
        IOLoop::Instance()->removeHandler(fd);
        return -1;
    }

    if (e.events & EPOLLERR) {
        return -1;
    }

    if (e.events & EPOLLOUT)
    {
        if (received > 0)
        {
            std::cout << "Writing: " << buffer << std::endl;
            if (send(fd, buffer, received, 0) != received)
            {
                std::cout << "Error writing to socket" << std::endl;
            }
        }

        IOLoop::Instance()->modifyHandler(fd, EPOLLIN);
    }

    if (e.events & EPOLLIN)
    {
        std::cout << "read" << std::endl;
        received = recv(fd, buffer, BUFFER_SIZE, 0);
        if (received < 0) {
            std::cout << "Error reading from socket" << std::endl;
        }
        else if (received > 0) {
            buffer[received] = 0;
            std::cout << "Reading: " << buffer << std::endl;
            if (strcmp(buffer, "stop") == 0) {
                std::cout << "stop----" << std::endl;
            }
        }

        if (received > 0) {
            IOLoop::Instance()->modifyHandler(fd, EPOLLOUT);
        } else {
            std::cout << "disconnect fd=" << fd << std::endl;
            IOLoop::Instance()->removeHandler(fd);
        }
    }

    return 0;
}

ServerHandler::ServerHandler(int port) {
    int fd;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        std::cout << "Failed to create server socket" << std::endl;
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        std::cout << "Failed to bind server socket" << std::endl;
        exit(1);
    }

    if (listen(fd, MAX_PENDING) < 0)
    {
        std::cout << "Failed to listen on server socket" << std::endl;
        exit(1);
    }
    setnonblocking(fd);

    IOLoop::Instance()->addHandler(fd, this, EPOLLIN);
}

int ServerHandler::handle(epoll_event e) {
    int fd = e.data.fd;
    struct sockaddr_in client_addr;
    socklen_t ca_len = sizeof(client_addr);

    int client = accept(fd, (struct sockaddr*)&client_addr, &ca_len);
    IOLoop::Instance()->addHandler(fd, this, EPOLLIN);
    if (client < 0)
    {
        std::cout << "Error accepting connection" << std::endl;
        return -1;
    }
    sockaddr_in sa;
    socklen_t len2 = sizeof(sa);
    if(!getpeername(client, (struct sockaddr *)&sa, &len2))
    {
        printf("accept new connect ,fd is %d, %s:%d\n", client, inet_ntoa(sa.sin_addr),ntohs(sa.sin_port) );
    }
    setnonblocking(client);
    Handler* clientHandler = new SpliceEchoHandler();
    IOLoop::Instance()->addHandler(client, clientHandler, EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR );
    return 0;
}

void ServerHandler::setnonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


SpliceEchoHandler::SpliceEchoHandler() {
    pipe(pipefd);

    fcntl(pipefd[1], F_SETPIPE_SZ, 1048576);
    fcntl(pipefd[0], F_SETPIPE_SZ, 1048576);
    int flags = fcntl(pipefd[0], F_GETPIPE_SZ);
    printf("pipe size %d\n", flags);
//    int flags = fcntl(pipefd[1], F_GETP);
//    fcntl(pipefd[1], F_SETFL, flags | O_NONBLOCK);
//    flags = fcntl(pipefd[0], F_GETFL);
//    fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);
}

SpliceEchoHandler::~SpliceEchoHandler() noexcept {
    received = 0;
    close(pipefd[0]);
    close(pipefd[1]);
}
int SpliceEchoHandler::handle(epoll_event e) {
    int fd = e.data.fd;
    if (e.events & EPOLLHUP) {
        std::cout << "close socket" << std::endl;
        IOLoop::Instance()->removeHandler(fd);
        close(fd);
        return -1;
    }

    if (e.events & EPOLLERR) {
        std::cout << "Error reading from socket" << std::endl;
        IOLoop::Instance()->removeHandler(fd);
        close(fd);
        return -1;
    }

    if (e.events & EPOLLOUT)
    {
//        if (received > 0)
//        {
//            received -= splice(pipefd[0],NULL,fd,NULL,received,SPLICE_F_MORE| SPLICE_F_NONBLOCK);
//        }
        received -= splice(pipefd[0],NULL,fd,NULL,received,SPLICE_F_MORE| SPLICE_F_NONBLOCK| SPLICE_F_MOVE);
        IOLoop::Instance()->modifyHandler(fd, EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR);
    }

    if (e.events & EPOLLIN)
    {
        received += splice(fd,NULL,pipefd[1],NULL,_data_len,SPLICE_F_MORE| SPLICE_F_MOVE  | SPLICE_F_NONBLOCK);
        if (received < 0) {
            std::cout << "Error reading from socket" << std::endl;
            IOLoop::Instance()->removeHandler(fd);
        } else if (received == 0){
            std::cout << " read close socket" << std::endl;
            IOLoop::Instance()->removeHandler(fd);
        } else{
            IOLoop::Instance()->modifyHandler(fd, EPOLLOUT | EPOLLET | EPOLLHUP | EPOLLERR);
        }
//        else if (received > 0) {
//            IOLoop::Instance()->modifyHandler(fd, EPOLLOUT | EPOLLET);
//        }else {
//            received = 1;
//        }
    }

    return 0;
}