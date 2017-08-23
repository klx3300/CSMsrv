#include "protocol.h"
#include "../zhwkre/serialization.h"
#include <stdlib.h>
#include <string.h>

// init dst using memcpy from bss
#define MEMCPYINIT(dst,bss) memcpy(&(dst),(bss).str,sizeof(dst))
// init UniversalHeader as hd
#define UHEADERINIT(qid,qname) UniversalHeader hd;hd.size=sizeof(hd.queryid)+sizeof(qname);hd.queryid=qid;
// copy to dst(pointer) from bss
#define BSSSET(dst,bss) memcpy(dst,(bss).str,(bss).size)
// append data to bss,auto sizeof and pointercast
#define BSSAPP(bss,cont) qbss_append((bss),(char*)(&cont),sizeof(cont))

binary_safe_string qAssembleLoginQuery(binary_safe_string username,binary_safe_string password){
    binary_safe_string bss;
    if(username.size > 256 || password.size > 256){
        return bss;// failed.
    }
    // construct universalHeader
    UHEADERINIT(0,LoginQuery);
    LoginQuery lq;
    lq.username_len = username.size;
    BSSSET(lq.username,username);
    lq.password_len = password.size;
    BSSSET(lq.password,password);
    BSSAPP(bss,hd);
    BSSAPP(bss,lq);
    return bss;
}

// not include the UniversalHeader
LoginQuery qDisassembleLoginQuery(binary_safe_string input){
    LoginQuery lq;
    MEMCPYINIT(lq,input);
    return lq;
}

binary_safe_string qAssembleLoginReply(unsigned int errNo,unsigned int userId,unsigned int groupId){
    binary_safe_string bss;
    UHEADERINIT(0,LoginReply);
    LoginReply lr;
    lr.errNo = errNo;
    lr.userId = userId;
    lr.groupId = groupId;
    BSSAPP(bss,hd);
    BSSAPP(bss,lr);
    return bss;
}

LoginReply qDisassembleLoginReply(binary_safe_string input){
    LoginReply lr;
    MEMCPYINIT(lr,input);
    return lr;
}

binary_safe_string qAssembleAlterPassQuery(unsigned int uid,unsigned int passlen,binary_safe_string password){
    binary_safe_string bss;
    if(password.size > 256){
        return bss;// failed;
    }
    UHEADERINIT(1,AlterPassQuery);
    AlterPassQuery apq;
    apq.userId = uid;
    apq.passlen = password.size;
    BSSSET(apq.newpass,password);
    BSSAPP(bss,hd);
    BSSAPP(bss,apq);
    return bss;
}

AlterPassReply qDisassembleAlterPassQuery(binary_safe_string bss){
    AlterPassReply apr;
    MEMCPYINIT(apr,bss);
    return apr;
}

binary_safe_string qAssembleListGroupQuery(unsigned int uid){
    binary_safe_string bss;
    UHEADERINIT(2,ListGroupQuery);
    ListGroupQuery lgq;
    lgq.userId = uid;
    BSSAPP(bss,hd);
    BSSAPP(bss,lgq);
    return bss;
}

ListGroupQuery qDisassembleListGroupQuery(binary_safe_string input){
    ListGroupQuery lgq;
    MEMCPYINIT(lgq,input);
    return lgq;
}

binary_safe_string qAssembleListGroupReply(qListDescriptor grouplist){
    binary_safe_string bss;
    UHEADERINIT(2,ListGroupReply);
    qListDescriptor ld = qSerialize(&grouplist,sizeof(qListDescriptor));
    binary_safe_string *serdata = ld.head->data;
    hd.size += serdata->size;
    ListGroupReply lgr;
    lgr.datasize = serdata->size;
    BSSAPP(bss,hd);
    BSSAPP(bss,lgr);
    q__bss_append(&bss,serdata->str,serdata->size);
    qbss_destructor(*serdata);
    qList_destructor(ld);
    return bss;
}

qListDescriptor* qDisassembleListGroupReply(binary_safe_string input){
    ListGroupReply lgr;
    MEMCPYINIT(lgr,input);
    char* ref = input.str+sizeof(ListGroupReply);
    qListDescriptor tmpdataset;
    qList_initdesc(tmpdataset);
    binary_safe_string tmpunser=qbss_new();
    q__bss_append(&tmpunser,ref,lgr.datasize);
    qList_push_back(tmpdataset,tmpunser);
    qListDescriptor* tmpdesc = qUnserialize(tmpdataset,YES_IT_IS_A_LIST);
    qbss_destructor(tmpunser);
    qList_destructor(tmpdataset);
    return tmpdesc;
}