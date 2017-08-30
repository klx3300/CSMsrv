#include "clinet.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../zhwkre/network.h"
#include "../advanced_network/advanced_network.h"
#include "../protocol/protocol.h"

qListDescriptor ui_notifier,network_notifier;
qMutex ui_noti_lock,net_noti_lock;

typedef unsigned int ui;
typedef unsigned char uc;

#define NETWRCHECK(sock,bss) do{\
    if(!qNetwork_writebss_auto((sock),(bss))){\
    fprintf(stderr,"[WARN] Network error occurrred while writing to sockfd %d\n",(sock).desc);}}while(0);

ui FIRST_TIME = 1;
ui RUNNING = 1;

void* receive_network(void* params){
    qSocket sock = *(qSocket*)params;
    while(RUNNING){
        ui qid = 0;
        binary_safe_string rcontent = qNetwork_readbss(sock,&qid);
        if(rcontent.size == 0){
            fprintf(stderr,"[INFO] Detected conn close.\n");
            qSocket_close(sock);
            LOCKUI;
            Messeage tmpmsg;
            tmpmsg.qid = 255;
            qList_push_back(ui_notifier,tmpmsg);
            UNLOCKUI;
            RUNNING = 0;
            continue;
        }
        Messeage msgr;
        msgr.qid = qid;
        msgr.payload = rcontent;
        LOCKUI;
        qList_push_back(ui_notifier,msgr);
        UNLOCKUI;
    }
    return NULL;
}
   
void* handle_network(void* params){
    qSocket sock;
    while(RUNNING){
        LOCKNET;
        if(network_notifier.size != 0){
            Messeage *curr = network_notifier.head->data;
            UniversalHeader *uh = (UniversalHeader*)curr->payload.str;
            fprintf(stderr,"[CURR%u]Attempt to send request len %u qid %u.\n",curr->qid,uh->size,uh->queryid);
            switch(curr->qid){
                case 0:
                {
                    // login
                    LoginAlter *la = (LoginAlter*)curr->payload.str;
                    if(FIRST_TIME){
                        sock = qSocket_constructor(qIPv4,qStreamSocket,qDefaultProto);
                        qSocket_open(sock);
                        if(!qStreamSocket_connect(sock,la->srv)){
                            fprintf(stderr,"Network error while trying to connect server.\n");
                        }
                        qRun(receive_network,&sock);
                        FIRST_TIME = 0;
                    }
                    NETWRCHECK(sock,la->querycont);
                }
                break;
                case 1:
                {
                    // normal query
                    NETWRCHECK(sock,curr->payload);
                }
                break;
                default:
                RUNNING = 0;
                break;
            }
            qList_pop_front(network_notifier);
        }
        UNLOCKNET;
    }
    fprintf(stderr,"Network handler stopped.\n");
    return NULL;
}