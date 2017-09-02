#include "advanced_network.h"
#include "../protocol/protocol.h"
#include <stdlib.h>
#include <string.h>

int qNetwork_read(qSocket sock,char* buffer,unsigned int length){
    unsigned int have_read=0;
    while(have_read<length){
        int onetime = qStreamSocket_read(sock,buffer,length-have_read);
        if(onetime == -1 || onetime == 0){
            return 0;
        }
        have_read += onetime;
        buffer+=onetime;
    }
    return 1;
}

binary_safe_string qNetwork_readbss(qSocket sock,unsigned int *queryid){
    binary_safe_string tmpbss=qbss_new();
    // first,read universalheader
    UniversalHeader uh;
    if(!qNetwork_read(sock,(char*)&uh,sizeof(uh))){
        return tmpbss;
    }
    *queryid = uh.queryid;
    uh.size -= sizeof(uh.queryid);
    char* tmpbuffer = malloc(uh.size);
    if(!qNetwork_read(sock,tmpbuffer,uh.size)){
        return tmpbss;
    }
    q__bss_append(&tmpbss,tmpbuffer,uh.size);
    free(tmpbuffer);
    return tmpbss;
}

int qNetwork_writebss_auto(qSocket sock,binary_safe_string bss){
    int val = qStreamSocket_write(sock,bss.str,bss.size);
    qbss_destructor(bss);
    return (val!=-1);
}