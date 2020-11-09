//
// Created by weiwei on 2020/10/28.
//

#include "IoContext.h"
#include <cstdlib>
#include <cstring>
#include <functional>
#include <event2/listener.h>
#include <memory>

IoContext::IoContext(int threads) :threads(threads){
    base_array = new event_base *[threads];
    for (int i = 0; i < threads; ++i) {
        base_array[i] = event_base_new();
    }
    if (threads == 1) {

    } else {

    }
}

event_base *IoContext::main_base() {
    return base_array[0];
}

event_base *IoContext::io_base() {
    if (threads == 1) {
        return base_array[0];
    }
    if (chose_io_index >= threads) {
        chose_io_index = 1;
    }
    return base_array[chose_io_index++];
}

void IoContext::dispatch() {
    if (threads > 1) {
        // TODO 多线程分发
    }
    event_base_dispatch(main_base());
    free();
}

void IoContext::free() {
    for (int i = 0; i < threads; ++i) {
        event_base_free(base_array[i]);
    }
}

IoContext::~IoContext() {
    delete[] base_array;
}


static void __evconnlistener_cb(struct evconnlistener *, evutil_socket_t s_fd, struct sockaddr * s_addr, int socklen, void * v){
    auto context = reinterpret_cast<IoContext*>(v);
    context->acceptCb(s_fd, s_addr, socklen, nullptr);
}

int IoContext::listen(const std::string &address_str, IoContext::accept_cb cb) {
    this->acceptCb = cb;
    this->address = address_str;
    memset(&listen_on_addr, 0, sizeof(listen_on_addr));
    int socklen = sizeof(listen_on_addr);

    char a[100];
    strcpy(a, address_str.c_str());
    const char *split = "*";
    auto sip = strtok(a, split);
    auto sport = strtok(nullptr, split);
    if (evutil_parse_sockaddr_port(address.c_str(),
                                   (struct sockaddr *) &listen_on_addr, &socklen) < 0) {
        int p = atoi(sport);
        auto *sin = (struct sockaddr_in *) &listen_on_addr;
        if (p < 1 || p > 65535) {
        }
        sin->sin_port = htons(p);
        sin->sin_addr.s_addr = htonl(0x7f000001);
        sin->sin_family = AF_INET;
        socklen = sizeof(struct sockaddr_in);
    }



    auto listener = evconnlistener_new_bind(this->main_base(), __evconnlistener_cb,
                                            this, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC | LEV_OPT_REUSEABLE | LEV_OPT_LEAVE_SOCKETS_BLOCKING,
                                            -1, (struct sockaddr *) &listen_on_addr, socklen);

    if (!listener) {
        fprintf(stderr, "Couldn't open listener.\n");
        return 1;
    }
    return 0;
}


