#include "clinet.h"
#include <stdlib.h>
#include <string.h>

qListDescriptor ui_notifier,network_notifier;
qMutex ui_noti_lock,net_noti_lock;

typedef unsigned int ui;
typedef unsigned char uc;

void* handle_network(void* params){
    ui RUNNING = 1;
    while(RUNNING){

    }
    return NULL;
}