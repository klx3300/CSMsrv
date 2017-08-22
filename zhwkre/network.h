#ifndef Q_RE_NETWORK_H
#define Q_RE_NETWORK_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/tcp.h>
#include <time.h>

const int qStreamSocket = SOCK_STREAM;
const int qDatagramSocket = SOCK_DGRAM;
const int qDefaultProto = 0;
const int qDatagramSocketDefaultFlag = 0;
const int qUnixDomain = AF_UNIX;
const int qIPv4 = AF_INET;
const int qIPv6 = AF_INET6;

struct q__Socket{
    int desc;
    int domain;
    int type;
    int protocol;
    int quickack;
};

typedef struct q__Socket qSocket;

// some useful util funcs
struct in_addr str_to_ipv4addr(const char* addrxport);

// generic for all socks
qSocket qSocket_constructor(int domain,int type,int protocol);
#define qSocket_open(sock) qSocket__open(&sock)
void qSocket__open(qSocket* sock);
void qSocket_bind(qSocket sock,const char* addr);
#define qSocket_close(sock) qSocket__close(&sock)
void qSocket__close(qSocket* sock);
#define qSocket_destructor(sock) qSocket__destructor(&sock)
void qSocket__destructor(qSocket* scok);

// stream sockets (tcp)
void qStreamSocket_connect(qSocket sock,const char* addrxport);
void qStreamSocket_listen(qSocket sock);
qSocket qStreamSocket_accept(qSocket sock,char* srcaddr);
//void qStreamSocket__sendbeat(qSocket sock);
//void qStreamSocket__acceptbeat(qSocket sock);
#define qStreamSocket_setQuickAck(sock,qack) qStreamSocket__setQuickAck(&sock,qack)
void qStreamSocket__setQuickAck(qSocket *sock,int quickack);
int qStreamSocket_write(qSocket sock,const char* content,unsigned int length);
int qStreamSocket_read(qSocket sock,char* buffer,unsigned int limitation);
int qStreamSocket_readchar(qSocket sock,char* c);
int qStreamSocket_nonblockRead(qSocket sock,char* buffer,unsigned int limitation);
int qStreamSocket_nonblockReadChar(qSocket sock,char *c);

// datagram sockets (udp)
int qDatagramSocket_receive(qSocket sock,char* srcaddr,char* buffer,unsigned int limitation,int flags);
int qDatagramSocket_send(qSocket sock,const char* dest,const char* content,unsigned int limitation,int flags);

#endif