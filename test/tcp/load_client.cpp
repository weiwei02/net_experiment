//
// Created by weiwei on 2020/11/3.
//

#include <assert.h>
#include <sys/socket.h>
#include <event.h>
#include <unistd.h>
#include <thread>
#include <cstring>
#include <iostream>

using namespace std;
static sockaddr_storage _addr;
static int _len;
static int _data_len = 4 * 1024 * 1024;
static char* _data;




static void write_data(bufferevent *b_out, void *base);
static void
eventcb(struct bufferevent *bev, short what, void *ctx){
    if (what & (BEV_EVENT_EOF|BEV_EVENT_ERROR)) {
        if (what & BEV_EVENT_ERROR) {
            if (errno)
                perror("connection error");
        }
    }
    if (what & BEV_EVENT_CONNECTED){
        write_data(bev, ctx);
    }
}
static void readcb(bufferevent *b_out, void *base){
    auto src = bufferevent_get_input(b_out);
    if (src)
        evbuffer_drain(src, evbuffer_get_length(src));

}
static void writecb(bufferevent *b_out, void *base){
    auto dst = bufferevent_get_output(b_out);
    evbuffer_add(dst, _data, _data_len);
}

static void write_data(bufferevent *b_out, void *base){
    auto dst = bufferevent_get_output(b_out);
    evbuffer_add(dst, _data, _data_len);
    bufferevent_setcb(b_out, readcb, writecb, nullptr, base);
//    bufferevent_enable(b_out, EV_WRITE|EV_READ);
//    bufferevent_setwatermark(b_out, EV_READ, 1024,
//                             _data_len);
    bufferevent_setwatermark(b_out, EV_WRITE, _data_len/2,
                             _data_len);
}


void *init_client(void*){
    event_base* base = event_base_new();
    if (!base) {
        perror("event_base_new()");
        exit(1);
    }
    bufferevent *b_out = bufferevent_socket_new(base, -1,BEV_OPT_CLOSE_ON_FREE);

    auto* a = static_cast<sockaddr *>(malloc(sizeof(sockaddr)));
    memset(a, 0, sizeof(sockaddr));
    memcpy(a, &_addr, _len);
    if (bufferevent_socket_connect(b_out, a, _len)<0) {
        perror("bufferevent_socket_connect");
        bufferevent_free(b_out);
        return nullptr;
    }
    bufferevent_setcb(b_out, readcb, write_data, eventcb, base);
    bufferevent_enable(b_out, EV_WRITE|EV_READ | EV_PERSIST | EV_ET);
    int no = event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY );
//     = event_base_dispatch(base);
    event_base_free(base);
    printf("done %d\n", no);
}

/**
 * load_client 127.0.0.1:5000 300
 * @1 服务端地址
 * @2 压测时间
 * */
int main(int argc, char** argv){
    assert(argc >= 3);
    cout << "use server address " << argv[1] << " ,test time is " << argv[2] << "seconds." << endl ;
    _len = sizeof(sockaddr_storage);
    memset(&_addr, 0, _len);
    if (evutil_parse_sockaddr_port(argv[1],
                                   (struct sockaddr*)&_addr, &_len) < 0){
        printf("ip:pot error");
        return 1;
    }
    _data = static_cast<char *>(malloc(_data_len));
    memset(_data, '0', _data_len);
    _data[_data_len - 1] = '\0';

//    init_client();
    unsigned int ngx_ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t tids[ngx_ncpu];
    pthread_attr_t attr;
    void *status;
    // 初始化并设置线程为可连接的（joinable）
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (int i = 0; i < ngx_ncpu; i++){
        pthread_create(&tids[i],&attr, init_client, nullptr);
    }
    // 删除属性，并等待其他线程
    pthread_attr_destroy(&attr);

    auto sec = atoi(argv[2]);
    sleep(sec);
//    for (int i = 0; i < ngx_ncpu; i++){
//        auto rc = pthread_join(tids[i], &status);
//        if (rc){
//            cout << "Error:unable to join," << rc << " status = " << status << endl;
//            exit(-1);
//        }
//        cout << "Main: completed thread id :" << i ;
//        cout << "  exiting with status :" << status << endl;
//    }
    printf("exit");

}
//
//
//
//int main(int argc, char** argv){
//    assert(argc >= 2);
//
//    _len = sizeof(sockaddr_storage);
//    memset(&_addr, 0, _len);
//    if (evutil_parse_sockaddr_port(argv[1],
//                                   (struct sockaddr*)&_addr, &_len) < 0){
//        printf("ip:pot error");
//        return 1;
//    }
//    _data = static_cast<char *>(malloc(_data_len));
//    memset(_data, '0', _data_len);
//    _data[_data_len - 1] = '\0';
//
////    init_client();
//    unsigned int ngx_ncpu = sysconf(_SC_NPROCESSORS_ONLN);
//    std::thread tids[ngx_ncpu];
//    for (int i = 0; i < ngx_ncpu; i++){
//        tids[i] = std::thread(init_client);
//    }
//    for (auto &thread : tids)
//        thread.join();
//    printf("exit");
//
//}