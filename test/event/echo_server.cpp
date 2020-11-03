//
// Created by weiwei on 2020/10/26.
//

#include "echo_server.h"
#include <iostream>
#include <cstring>

#include <errno.h>

#include "event.h"
#include "event2/bufferevent.h"

using namespace std;

int tcp_server_init2(int);
void accept_cb2(int fd, short events, void *arg);
void sockert_read_cb2(struct bufferevent *bev, void *arg);
void event_cb2(struct bufferevent *bev, short events, void *arg);

int buffer_server(int port){
    int listener = tcp_server_init2(port);
    if (listener == -1)
    {
        cout << "tcp server init error ";
        return -1;
    }
    struct event_base *base = event_base_new();

    // 添加客户端请求连接事件
    struct event * ev_listen = event_new(base, listener, EV_READ| EV_PERSIST, accept_cb2, base);
    event_add(ev_listen, nullptr);
    event_base_dispatch(base);
    event_base_free(base);
    return 0;
}

int main(){
    buffer_server(8899);
}

/**
 * 接收新网络连接方法
*/
void accept_cb2(int fd, short events, void *arg){
    evutil_socket_t sockfd;
    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    sockfd = accept(fd, (struct sockaddr*)&client, &len);
    evutil_make_socket_nonblocking(sockfd);
//    cout << "accept a client " << sockfd << endl;
    struct event_base* base = (event_base*)arg;

    bufferevent* bev = bufferevent_socket_new(base, sockfd, BEV_OPT_CLOSE_ON_FREE);
    /*bufferevent_setcb()函数修改 bufferevent 的一个或者多个回调 。
    readcb、writecb和eventcb函数将分别在已经读取足够的数据 、已经写入足够的数据 ,或者发生错误时被调用 。
每个回调函数的第一个参数都是发生了事件的bufferevent ,最后一个参数都是调用bufferevent_setcb()时用户提供
的 cbarg 参数:可以通过它向回调传递数据。事件回调 的 events 参数是一个表示事件标志的位掩码。
要禁用回调,传递 NULL 而不是回调函数 。注意:bufferevent 的所有回调函数共享单 个 cbarg, 所以修改它将影
响所有回调函数。*/
    bufferevent_setcb(bev, sockert_read_cb2, nullptr, event_cb2, arg);

    bufferevent_enable(bev, EV_READ | EV_PERSIST);
}

void sockert_read_cb2(struct bufferevent *bev, void *arg){
    char msg[4096];
    size_t len = bufferevent_read(bev, msg, sizeof(msg));
    msg[len] = '\0';

    cout << endl << msg << endl;
    bufferevent_write(bev, msg, len);
}

void event_cb2(struct bufferevent *bev, short event, void *arg){
    if (event & BEV_EVENT_EOF)
    {
        cerr << "connection closed" << endl;
    }else if (event & BEV_EVENT_ERROR){
        cerr << "some other error" << endl;
    }
    // 自动关闭缓冲区和套接字
    bufferevent_free(bev);
}

int tcp_server_init2(int port){
    int errno_save;
    evutil_socket_t listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1){
        return -1;
    }

    // 允许多次绑定同一个地址，要用在socket和bind之间
    evutil_make_listen_socket_reuseable(listener);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(port);

    if(bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0){
        goto error;
    }
    if(listen(listener, 100) < 0){
        goto error;
    }

    evutil_make_socket_nonblocking(listener);
    return listener;


    error:
    errno_save = errno;
    evutil_closesocket(listener);
    errno = errno_save;
    return -1;
}