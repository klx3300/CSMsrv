#ifndef Q_CDES_ADV_NETWORK_H
#define Q_CDES_ADV_NETWORK_H

#include "../zhwkre/network.h"
#include "../zhwkre/bss.h"

// return false:sock_read error. true:read complete.
int qNetwork_read(qSocket sock,char* buffer,unsigned int length);

// binary_safe_string.size=0 => sock_read error.
binary_safe_string qNetwork_readbss(qSocket sock,unsigned int *queryid);

int qNetwork_writebss_auto(qSocket sock,binary_safe_string bss);

#endif