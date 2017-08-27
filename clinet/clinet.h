#ifndef Q_CDES_CLI_NETWORK_H
#define Q_CDES_CLI_NETWORK_H
#include "../configure/stdafx.h"

#include "../zhwkre/bss.h"
#include "../zhwkre/list.h"
#include "../zhwkre/concurrent.h"

typedef struct q_Messeage_st{unsigned char qid;binary_safe_string payload;} Messeage;

typedef struct q_LoginAlter_st{char srv[256];binary_safe_string querycont;} LoginAlter;

void* handle_network(void* params);
extern qListDescriptor ui_notifier,network_notifier;
extern qMutex ui_noti_lock,net_noti_lock;
#define LOCKUI ui_noti_lock.lock(ui_noti_lock)
#define UNLOCKUI ui_noti_lock.unlock(ui_noti_lock)
#define LOCKNET net_noti_lock.lock(net_noti_lock)
#define UNLOCKNET net_noti_lock.unlock(net_noti_lock)

#endif