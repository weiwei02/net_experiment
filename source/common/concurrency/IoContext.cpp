//
// Created by weiwei on 2020/10/28.
//

#include "IoContext.h"
#include <cstdlib>

IoContext::IoContext(int threads) {
    base_array = new event_base* [threads];
    for (int i = 0; i < threads; ++i) {
        base_array[i] = event_base_new();
    }
    if (threads == 1){

    }else {

    }
}

event_base *IoContext::main_base() {
    return base_array[0];
}

event_base *IoContext::io_base() {
    if (threads == 1){
        return base_array[0];
    }
    if (chose_io_index >= threads){
        chose_io_index = 1;
    }
    return base_array[chose_io_index++];
}

void IoContext::dispatch() {
    if (threads > 1){
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
    delete [] base_array;
}
