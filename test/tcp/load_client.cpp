//
// Created by weiwei on 2020/11/3.
//

#include <assert.h>
#include <bits/socket.h>
#include <event.h>
#include <unistd.h>
#include <thread>
#include <cstring>

static sockaddr _addr;
static int _len;
static int _data_len = 4 * 1024 * 1024;
static char* _data;

static void readcb(bufferevent *b_out, void *base);
static void writecb(bufferevent *b_out, void *base);

static void write_data(bufferevent *b_out, void *base){
    auto dst = bufferevent_get_output(b_out);
    evbuffer_add(dst, _data, _data_len);
    bufferevent_setcb(b_out, readcb, writecb, nullptr, base);
    bufferevent_enable(b_out, EV_WRITE|EV_READ);
    bufferevent_setwatermark(b_out, EV_READ, 1024,
                             _data_len);
}
void init_client(void *p){
    event_base* base = event_base_new();
    if (!base) {
        perror("event_base_new()");
        exit(1);
    }
    bufferevent *b_out = bufferevent_socket_new(base, -1,
                                                       BEV_OPT_CLOSE_ON_FREE|BEV_OPT_DEFER_CALLBACKS);
    if (bufferevent_socket_connect(b_out,
                                   (struct sockaddr*)&_addr, _len)<0) {
        perror("bufferevent_socket_connect");
        bufferevent_free(b_out);
        return;
    }
    bufferevent_setcb(b_out, nullptr, write_data, nullptr, base);
}


int main(int argc, char** argv){
    assert(argc >= 1);

    if (evutil_parse_sockaddr_port(argv[1],
                                   (struct sockaddr*)&_addr, &_len) < 0){
        printf("ip:pot error");
        return 1;
    }
    memset(_data, '0', _data_len);
    _data[_data_len - 1] = '\0';


    for (int i = 0; i < _data_len; i++){

    }

    unsigned int ngx_ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    std::thread tids[ngx_ncpu];
    for (int i = 0; i < ngx_ncpu; i++){
        tids[i] = std::thread(init_client, nullptr);
    }
    for (int i = 0; i < ngx_ncpu; i++){
        tids[i].join();
    }
}