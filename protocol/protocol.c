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
    binary_safe_string bss=qbss_new();
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
    binary_safe_string bss=qbss_new();
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
    binary_safe_string bss=qbss_new();
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
    binary_safe_string bss=qbss_new();
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

binary_safe_string qAssembleListGroupReply(unsigned int errno,qListDescriptor grouplist){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(2,ListGroupReply);
    qListDescriptor ld = qSerialize(&grouplist,sizeof(qListDescriptor));
    binary_safe_string *serdata = ld.head->data;
    hd.size += serdata->size;
    ListGroupReply lgr;
    lgr.errNo = errno;
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
    if(lgr.errNo != 0){
        return NULL;
    }
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

binary_safe_string qAssembleAlterGroupQuery(unsigned int uid,unsigned int gid){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(3,AlterGroupQuery);
    AlterGroupQuery agq;
    agq.userId = uid;
    agq.groupId = gid;
    BSSAPP(bss,hd);
    BSSAPP(bss,agq);
    return bss;
}

AlterGroupQuery qDisassembleAlterGroupQuery(binary_safe_string input){
    AlterGroupQuery agq;
    MEMCPYINIT(agq,input);
    return agq;
}

binary_safe_string qAssembleAlterGroupReply(unsigned int errno){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(3,AlterGroupReply);
    AlterGroupReply agr;
    agr.errNo = errno;
    BSSAPP(bss,hd);
    BSSAPP(bss,agr);
    return bss;
}

AlterGroupReply qDisassembleAlterGroupReply(binary_safe_string bss){
    AlterGroupReply agr;
    MEMCPYINIT(agr,bss);
    return agr;
}

binary_safe_string qAssembleRemoveUserQuery(unsigned int uid,unsigned int destuid){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(4,RemoveUserQuery);
    RemoveUserQuery ruq;
    ruq.userId = uid;
    ruq.targetUserId = destuid;
    BSSAPP(bss,hd);
    BSSAPP(bss,ruq);
    return bss;
}

RemoveUserQuery qDisassembleRemoveUserQuery(binary_safe_string input){
    RemoveUserQuery ruq;
    MEMCPYINIT(ruq,input);
    return ruq;
}

binary_safe_string qAssembleRemoveUserReply(unsigned int errno){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(4,RemoveUserReply);
    RemoveUserReply agr;
    agr.errNo = errno;
    BSSAPP(bss,hd);
    BSSAPP(bss,agr);
    return bss;
}

RemoveUserReply qDisassembleRemoveUserReply(binary_safe_string input){
    RemoveUserReply rur;
    MEMCPYINIT(rur,input);
    return rur;
}

binary_safe_string qAssembleRemoveGroupQuery(unsigned int uid,unsigned int gid){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(5,RemoveGroupQuery);
    RemoveGroupQuery rgq;
    rgq.userId = uid;
    rgq.targetGroupId = gid;
    BSSAPP(bss,hd);
    BSSAPP(bss,rgq);
    return bss;
}

RemoveGroupQuery qDisassembleRemoveGroupQuery(binary_safe_string input){
    RemoveGroupQuery rgq;
    MEMCPYINIT(rgq,input);
    return rgq;
}

binary_safe_string qAssembleRemoveGroupReply(unsigned int errno){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(5,RemoveGroupReply);
    RemoveGroupReply agr;
    agr.errNo = errno;
    BSSAPP(bss,hd);
    BSSAPP(bss,agr);
    return bss;
}

RemoveGroupReply qDisassembleRemoveGroupReply(binary_safe_string input){
    RemoveGroupReply rur;
    MEMCPYINIT(rur,input);
    return rur;
}

binary_safe_string qAssembleListUserQuery(unsigned int uid){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(6,ListUserQuery);
    ListUserQuery lgq;
    lgq.uid = uid;
    BSSAPP(bss,hd);
    BSSAPP(bss,lgq);
    return bss;
}

ListUserQuery qDisassembleListUserQuery(binary_safe_string input){
    ListUserQuery lgq;
    MEMCPYINIT(lgq,input);
    return lgq;
}

binary_safe_string qAssembleListUserReply(unsigned int errno,qListDescriptor grouplist){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(6,ListUserReply);
    qListDescriptor ld = qSerialize(&grouplist,sizeof(qListDescriptor));
    binary_safe_string *serdata = ld.head->data;
    hd.size += serdata->size;
    ListUserReply lgr;
    lgr.errNo = errno;
    lgr.length = serdata->size;
    BSSAPP(bss,hd);
    BSSAPP(bss,lgr);
    q__bss_append(&bss,serdata->str,serdata->size);
    qbss_destructor(*serdata);
    qList_destructor(ld);
    return bss;
}

qListDescriptor* qDisassembleListUserReply(binary_safe_string input){
    ListUserReply lgr;
    MEMCPYINIT(lgr,input);
    if(lgr.errNo != 0) return NULL;
    char* ref = input.str+sizeof(ListUserReply);
    qListDescriptor tmpdataset;
    qList_initdesc(tmpdataset);
    binary_safe_string tmpunser=qbss_new();
    q__bss_append(&tmpunser,ref,lgr.length);
    qList_push_back(tmpdataset,tmpunser);
    qListDescriptor* tmpdesc = qUnserialize(tmpdataset,YES_IT_IS_A_LIST);
    qbss_destructor(tmpunser);
    qList_destructor(tmpdataset);
    return tmpdesc;
}

binary_safe_string qAssembleSyncDataQuery(unsigned int uid,unsigned int gid){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(10,SyncDataQuery);
    SyncDataQuery sdq;
    sdq.userId = uid;
    sdq.groupId = gid;
    BSSAPP(bss,hd);
    BSSAPP(bss,sdq);
    return bss;
}

SyncDataQuery qDisassembleSyncDataQuery(binary_safe_string input){
    SyncDataQuery sdq;
    MEMCPYINIT(sdq,input);
    return sdq;
}

binary_safe_string qAssembleSyncDataReply(unsigned int errNo,qListDescriptor ser_data){
    binary_safe_string bss=qbss_new();
    if(errNo != 0){
        UHEADERINIT(10,SyncDataReply);
        SyncDataReply sdr;
        sdr.errNo = errNo;
        sdr.levels = 0;
        BSSAPP(bss,hd);
        BSSAPP(bss,sdr);
        return bss;
    }
    UHEADERINIT(10,SyncDataReply);
    SyncDataReply sdr;
    binary_safe_string tmpbss=qbss_new();
    sdr.errNo = errNo;
    sdr.levels = ser_data.size;
    qList_foreach(ser_data,seri){
        binary_safe_string *tmpref = seri->data;
        q__bss_append(&tmpbss,(char*)&(tmpref->size),sizeof(unsigned int));
        q__bss_append(&tmpbss,tmpref->str,tmpref->size);
    }
    hd.size += tmpbss.size;
    BSSAPP(bss,hd);
    BSSAPP(bss,sdr);
    q__bss_append(&bss,tmpbss.str,tmpbss.size);
    qbss_destructor(tmpbss);
    return bss;
}

qListDescriptor qDisassembleSyncDataReply(binary_safe_string input){
    SyncDataReply sdr;
    MEMCPYINIT(sdr,input);
    char* refr = input.str+sizeof(SyncDataReply);
    if(sdr.errNo != 0){
        qListDescriptor ld;
        qList_initdesc(ld);
        return ld;
    }
    qListDescriptor ld;
    qList_initdesc(ld);
    for(int i=0;i<sdr.levels;i++){
        binary_safe_string tmpbss=qbss_new();
        unsigned int tmpsize = *(unsigned int*)refr;
        refr+=sizeof(unsigned int);
        q__bss_append(&tmpbss,refr,tmpsize);
        qList_push_back(ld,tmpbss);
    }
    return ld;
}

binary_safe_string qAssembleAppendDataQuery(unsigned int uid,unsigned int gid,unsigned int entryids[3],binary_safe_string data){
    binary_safe_string bss = qbss_new();
    UHEADERINIT(11,AppendDataQuery);
    hd.size += data.size;
    AppendDataQuery adq;
    adq.userId = uid;
    adq.groupId = gid;
    memcpy(adq.entryIds,entryids,3*sizeof(unsigned int));
    adq.datalen = data.size;
    BSSAPP(bss,hd);
    BSSAPP(bss,adq);
    q__bss_append(&bss,data.str,data.size);
    return bss;
}

AppendDataQuery *qDisassembleAppendDataQuery(binary_safe_string input){
    AppendDataQuery tmpadq;
    MEMCPYINIT(tmpadq,input);
    void *adq = malloc(sizeof(AppendDataQuery)+tmpadq.datalen);
    memcpy(adq,&tmpadq,sizeof(AppendDataQuery));
    memcpy(adq+sizeof(AppendDataQuery),input.str+sizeof(AppendDataQuery),tmpadq.datalen);
    return adq;
}

binary_safe_string qAssembleAppendDataReply(unsigned int errNo,unsigned int entryId){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(11,AppendDataReply);
    AppendDataReply adr;
    adr.errNo = errNo;
    adr.entryId = entryId;
    BSSAPP(bss,hd);
    BSSAPP(bss,adr);
    return bss;
}

AppendDataReply qDisassembleAppendDataReply(binary_safe_string input){
    AppendDataReply adr;
    MEMCPYINIT(adr,input);
    return adr;
}

binary_safe_string qAssembleAlterDataQuery(unsigned int uid,unsigned int gid,unsigned int entryids[3],binary_safe_string data){
    binary_safe_string bss = qbss_new();
    UHEADERINIT(13,AlterDataQuery);
    hd.size += data.size;
    AlterDataQuery adq;
    adq.userId = uid;
    adq.groupId = gid;
    memcpy(adq.entryIds,entryids,3*sizeof(unsigned int));
    adq.datalen = data.size;
    BSSAPP(bss,hd);
    BSSAPP(bss,adq);
    q__bss_append(&bss,data.str,data.size);
    return bss;
}

AlterDataQuery *qDisassembleAlterDataQuery(binary_safe_string input){
    AlterDataQuery tmpadq;
    MEMCPYINIT(tmpadq,input);
    void *adq = malloc(sizeof(AlterDataQuery)+tmpadq.datalen);
    memcpy(adq,&tmpadq,sizeof(AlterDataQuery));
    memcpy(adq+sizeof(AlterDataQuery),input.str+sizeof(AlterDataQuery),tmpadq.datalen);
    return adq;
}

binary_safe_string qAssembleAlterDataReply(unsigned int errNo){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(13,AlterDataReply);
    AlterDataReply adr;
    adr.errNo = errNo;
    BSSAPP(bss,hd);
    BSSAPP(bss,adr);
    return bss;
}

AlterDataReply qDisassembleAlterDataReply(binary_safe_string input){
    AlterDataReply adr;
    MEMCPYINIT(adr,input);
    return adr;
}

binary_safe_string qAssembleRemoveDataQuery(unsigned int uid,unsigned int gid,unsigned int entryids[3]){
    binary_safe_string bss = qbss_new();
    UHEADERINIT(12,RemoveDataQuery);
    RemoveDataQuery adq;
    adq.userId = uid;
    adq.groupId = gid;
    memcpy(adq.entryIds,entryids,3*sizeof(unsigned int));
    BSSAPP(bss,hd);
    BSSAPP(bss,adq);
    return bss;
}

RemoveDataQuery qDisassembleRemoveDataQuery(binary_safe_string input){
    RemoveDataQuery tmpadq;
    MEMCPYINIT(tmpadq,input);
    return tmpadq;
}

binary_safe_string qAssembleRemoveDataReply(unsigned int errNo){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(12,RemoveDataReply);
    RemoveDataReply adr;
    adr.errNo = errNo;
    BSSAPP(bss,hd);
    BSSAPP(bss,adr);
    return bss;
}

RemoveDataReply qDisassembleRemoveDataReply(binary_safe_string input){
    RemoveDataReply adr;
    MEMCPYINIT(adr,input);
    return adr;
}

binary_safe_string qAssembleAlterEntryOwnerQuery(unsigned int uid,unsigned int destuid,unsigned int entryids[3]){
    binary_safe_string bss = qbss_new();
    UHEADERINIT(20,AlterEntryOwnerQuery);
    AlterEntryOwnerQuery adq;
    adq.userId = uid;
    adq.destuid = destuid;
    memcpy(adq.entryIds,entryids,3*sizeof(unsigned int));
    BSSAPP(bss,hd);
    BSSAPP(bss,adq);
    return bss;
}

AlterEntryOwnerQuery qDisassembleAlterEntryOwnerQuery(binary_safe_string input){
    AlterEntryOwnerQuery tmpadq;
    MEMCPYINIT(tmpadq,input);
    return tmpadq;
}

binary_safe_string qAssembleAlterEntryOwnerReply(unsigned int errNo){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(20,AlterEntryOwnerReply);
    AlterEntryOwnerReply adr;
    adr.errNo = errNo;
    BSSAPP(bss,hd);
    BSSAPP(bss,adr);
    return bss;
}

AlterEntryOwnerReply qDisassembleAlterEntryOwnerReply(binary_safe_string input){
    AlterEntryOwnerReply adr;
    MEMCPYINIT(adr,input);
    return adr;
}

binary_safe_string qAssembleAlterEntryGroupQuery(unsigned int uid,unsigned int gid,unsigned int destgid,unsigned int entryids[3]){
    binary_safe_string bss = qbss_new();
    UHEADERINIT(21,AlterEntryGroupQuery);
    AlterEntryGroupQuery adq;
    adq.userId = uid;
    adq.groupId = gid;
    adq.destgid = destgid;
    memcpy(adq.entryIds,entryids,3*sizeof(unsigned int));
    BSSAPP(bss,hd);
    BSSAPP(bss,adq);
    return bss;
}

AlterEntryGroupQuery qDisassembleAlterEntryGroupQuery(binary_safe_string input){
    AlterEntryGroupQuery tmpadq;
    MEMCPYINIT(tmpadq,input);
    return tmpadq;
}

binary_safe_string qAssembleAlterEntryGroupReply(unsigned int errNo){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(21,AlterEntryGroupReply);
    AlterEntryGroupReply adr;
    adr.errNo = errNo;
    BSSAPP(bss,hd);
    BSSAPP(bss,adr);
    return bss;
}

AlterEntryGroupReply qDisassembleAlterEntryGroupReply(binary_safe_string input){
    AlterEntryGroupReply adr;
    MEMCPYINIT(adr,input);
    return adr;
}

binary_safe_string qAssembleAlterEntryPermissionQuery(unsigned int uid,unsigned char perm,unsigned int entryids[3]){
    binary_safe_string bss = qbss_new();
    UHEADERINIT(22,AlterEntryPermissionQuery);
    AlterEntryPermissionQuery adq;
    adq.userId = uid;
    adq.permission = perm;
    memcpy(adq.entryIds,entryids,3*sizeof(unsigned int));
    BSSAPP(bss,hd);
    BSSAPP(bss,adq);
    return bss;
}

AlterEntryPermissionQuery qDisassembleAlterEntryPermissionQuery(binary_safe_string input){
    AlterEntryPermissionQuery tmpadq;
    MEMCPYINIT(tmpadq,input);
    return tmpadq;
}

binary_safe_string qAssembleAlterEntryPermissionReply(unsigned int errNo){
    binary_safe_string bss=qbss_new();
    UHEADERINIT(22,AlterEntryPermissionReply);
    AlterEntryPermissionReply adr;
    adr.errNo = errNo;
    BSSAPP(bss,hd);
    BSSAPP(bss,adr);
    return bss;
}

AlterEntryPermissionReply qDisassembleAlterEntryPermissionReply(binary_safe_string input){
    AlterEntryPermissionReply adr;
    MEMCPYINIT(adr,input);
    return adr;
}
